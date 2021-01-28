#ifndef SYNAPSE_PROCESSING_H
#define SYNAPSE_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/edges.h>
#include <core/objects/transfer_objects.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/item_buffer.h>
#include <core/objects/synapses.h>
#include <core/objects/global_values.h>

//==================================================================================================

void
synapseProcessing(SynapseSection* synapseSection,
                  float weight,
                  float hardening)
{
    const uint64_t numberOfNodes = KyoukoRoot::m_segment->nodes.numberOfItems;
    Node* nodes = getBuffer<Node>(KyoukoRoot::m_segment->nodes);
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    uint32_t pos = 0;

    // iterate over all synapses in the section and update the target-nodes
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && weight > 2.0f)
    {
        Synapse* synapse = &synapseSection->synapses[pos];

        if(synapse->targetNodeId == UNINIT_STATE_16) {
            break;
        }

        const float newHardening = synapse->hardening + hardening;
        synapse->hardening = (newHardening > 1.0f) * 1.0f + (newHardening <= 1.0f) * newHardening;

        // update static weight value
        const float hardeningDiff = newHardening - synapse->hardening;
        const float diff = synapse->dynamicWeight * hardeningDiff;
        synapse->dynamicWeight -= diff;
        synapse->staticWeight += diff;

        hardening /= 2.0f;

        // 1 because only one thread at the moment
        const ulong nodeBufferPosition = (1 * (numberOfNodes / 256)) + synapse->targetNodeId;
        const float synapseWeight = synapse->staticWeight + synapse->dynamicWeight;
        const float shareWeight = (weight > synapseWeight) * synapseWeight
                                  + (weight <= synapseWeight) * weight;

        nodes[nodeBufferPosition].currentState += shareWeight * static_cast<float>(synapse->sign);
        weight -= shareWeight;
        pos++;
    }

    if(weight > 2.0f)
    {
        for(uint32_t i = pos; i < SYNAPSES_PER_SYNAPSESECTION; i++)
        {
            Synapse* synapse = &synapseSection->synapses[pos];

            const float random = (rand() % 1024) / 1024.0f;
            float usedLearn = (weight * random) + 1.0f;
            if(weight < 5.0f) {
                usedLearn = weight;
            }

            // get random node-id as target
            const uint32_t targetNodeIdInBrick = static_cast<uint32_t>(rand()) % globalValue->numberOfNodesPerBrick;

            // set initial values for the new synapse
            const uint32_t nodeOffset = synapseSection->nodeBrickId * globalValue->numberOfNodesPerBrick;
            synapse->targetNodeId = static_cast<uint16_t>(targetNodeIdInBrick + nodeOffset);
            synapse->hardening = 0.0f;

            // make to blocking synapse by random
            if(rand() % 2 == 0) {
                synapse->sign = -1;
            }

            synapse->dynamicWeight = usedLearn;
            weight -= usedLearn;
        }
    }
}

//==================================================================================================

void
synapseCreateProcessing()
{
    SynapseTransfer* synapseTransfers = getBuffer<SynapseTransfer>(KyoukoRoot::m_segment->synapseTransfers);
    SynapseSection* synapseSections = getBuffer<SynapseSection>(KyoukoRoot::m_segment->synapses);
    const uint64_t numberOfSynapseTransfers = KyoukoRoot::m_segment->synapseTransfers.numberOfItems;

    for(uint64_t i = 0; i < numberOfSynapseTransfers; i++)
    {
        const uint32_t nextSynapseSectionId = synapseTransfers[i].nextSynapseSectionId;
        if(nextSynapseSectionId == UNINIT_STATE_32) {
            continue;
        }

        // check if section is new and schould be created
        SynapseSection newSection;
        newSection.status = ACTIVE_SECTION;
        newSection.randomPos = rand() % 1024;
        newSection.isOutput = synapseTransfers[i].isOutput;
        synapseSections[nextSynapseSectionId] = newSection;

        synapseSections[nextSynapseSectionId].prev = synapseTransfers[i].prevSynapseSectionId;
        synapseSections[synapseTransfers[i].nextSynapseSectionId].next = nextSynapseSectionId;
    }
}

//==================================================================================================

void
updating(SynapseSection* synapseSection)
{
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    Node* nodes = getBuffer<Node>(KyoukoRoot::m_segment->nodes);
    SynapseSection* synapseSections = getBuffer<SynapseSection>(KyoukoRoot::m_segment->synapses);

    // iterate over all synapses in synapse-section
    uint32_t currentPos = 0;
    for(uint32_t lastPos = 0; lastPos < SYNAPSES_PER_SYNAPSESECTION; lastPos++)
    {
        Synapse* synapse = &synapseSection->synapses[lastPos];
        // skip unused synapse in section
        if(synapse->targetNodeId == UNINIT_STATE_16) {
            continue;
        }

        // update dynamic-weight-value of the synapse
        if(nodes[synapse->targetNodeId].active == 0) {
            synapse->dynamicWeight = synapse->dynamicWeight * globalValue->initialMemorizing;
        } else {
            synapse->dynamicWeight = synapse->dynamicWeight * 0.95f;
        }

        // check for deletion of the single synapse
        const float synapseWeight = synapse->dynamicWeight + synapse->staticWeight;
        if(synapseWeight < globalValue->deleteSynapseBorder)
        {
            synapse->dynamicWeight = 0.0f;
            synapse->staticWeight = 0.0f;
            synapse->targetNodeId = UNINIT_STATE_16;
            synapse->sign = 1;
        }
        else
        {
            synapseSection->synapses[currentPos] = synapseSection->synapses[lastPos];
            currentPos++;
        }
    }

    if(synapseSection->next != UNINIT_STATE_32) {
        updating(&synapseSections[synapseSection->next]);
    }
}

//==================================================================================================

void
node_processing()
{
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    Node* nodes = getBuffer<Node>(KyoukoRoot::m_segment->nodes);
    const uint64_t numberOfNodes = KyoukoRoot::m_segment->nodes.numberOfItems;
    SynapseSection* synapseSections = getBuffer<SynapseSection>(KyoukoRoot::m_segment->synapses);

    for(uint64_t i = 0; i < numberOfNodes / 256; i++)
    {
        // TODO: when port to gpu: change 2 to 256 again
        for(uint pos = 1; pos < 2; pos++)
        {
            const ulong nodeBufferPosition = (pos * (numberOfNodes / 256)) + i;
            nodes[i].currentState += nodes[nodeBufferPosition].currentState;
            nodes[nodeBufferPosition].currentState = 0.0f;
        }
    }

    for(uint64_t i = 0; i < numberOfNodes / 256; i++)
    {
        Node* node = &nodes[i];
        if(node->border > 0.0f)
        {
            // set to 255.0f, if value is too high
            const float cur = node->currentState;
            node->currentState = (cur > 255.0f) * 255.0f + (cur <= 255.0f) * cur;
            node->currentState = (cur < 0.0f) * 0.0f + (cur >= 0.0f) * cur;

            // check if active
            if(node->border < node->currentState
                    && node->refractionTime == 0)
            {
                node->potential = globalValue->actionPotential;
                node->refractionTime = globalValue->refractionTime;
            }

            if(node->potential > 10.0f)
            {
                node->active = 1;
                // build new axon-transfer-edge, which is send back to the host
                const float up = static_cast<float>(pow(globalValue->gliaValue,
                                                        node->targetBrickDistance));
                const float weight = node->potential * up;
                synapseProcessing(&synapseSections[i], weight, globalValue->lerningValue);
            }
            else
            {
                node->active = 0;
                updating(&synapseSections[i]);
            }

            // post-steps
            node->refractionTime = node->refractionTime >> 1;

            // set to 0.0f, if value is negative
            const float newCur = node->currentState;
            node->currentState = (newCur < 0.0f) * 0.0f + (newCur >= 0.0f) * newCur;

            // make cooldown in the node
            node->potential /= globalValue->nodeCooldown;
            node->currentState /= globalValue->nodeCooldown;
        }
    }
}

#endif // SYNAPSE_PROCESSING_H
