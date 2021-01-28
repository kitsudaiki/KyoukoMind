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

bool
findNewSectioin(const uint32_t oldSectionId)
{
    SynapseSection* synapseSections = getBuffer<SynapseSection>(KyoukoRoot::m_segment->synapses);
    const uint64_t numberOfSections = KyoukoRoot::m_segment->synapses.itemCapacity;

    for(uint32_t i = 0; i < numberOfSections; i++)
    {
        if(synapseSections[i].status == DELETED_SECTION)
        {
            // check if section is new and schould be created
            SynapseSection newSection;
            newSection.status = ACTIVE_SECTION;
            newSection.randomPos = rand() % 1024;
            synapseSections[i]= newSection;

            synapseSections[i].prev = oldSectionId;
            synapseSections[oldSectionId].next = i;
            return true;
        }
    }

    return false;
}

/**
 * @brief remove
 * @param pos
 * @return
 */
void
removeSection(const uint32_t pos)
{
    SynapseSection* synapseSections = getBuffer<SynapseSection>(KyoukoRoot::m_segment->synapses);

    SynapseSection* section = &synapseSections[pos];
    SynapseSection* prev = &synapseSections[section->prev];

    if(section->next != UNINIT_STATE_16)
    {
        SynapseSection* next = &synapseSections[section->next];
        next->prev = section->prev;
    }

    prev->next = section->next;

    SynapseSection emptyEdge;
    synapseSections[pos] = emptyEdge;
}

//--------------------------------------------------------------------------------------------------

inline void
synapseProcessing(const uint32_t sectionPos,
                  float weight,
                  float hardening)
{
    SynapseSection* synapseSections = getBuffer<SynapseSection>(KyoukoRoot::m_segment->synapses);
    SynapseSection* section = &synapseSections[sectionPos];

    const uint64_t numberOfNodes = KyoukoRoot::m_segment->nodes.numberOfItems;
    Node* nodes = getBuffer<Node>(KyoukoRoot::m_segment->nodes);
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    uint32_t pos = 0;

    // iterate over all synapses in the section and update the target-nodes
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && weight > 2.0f)
    {
        Synapse* synapse = &section->synapses[pos];

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
        while(pos < SYNAPSES_PER_SYNAPSESECTION)
        {
            Synapse* synapse = &section->synapses[pos];

            const float random = (rand() % 1024) / 1024.0f;
            float usedLearn = (weight * random) + 1.0f;
            if(weight < 5.0f) {
                usedLearn = weight;
            }

            // get random node-id as target
            const uint32_t targetNodeIdInBrick = static_cast<uint32_t>(rand()) % globalValue->numberOfNodesPerBrick;

            // set initial values for the new synapse
            const uint32_t nodeOffset = section->nodeBrickId * globalValue->numberOfNodesPerBrick;
            synapse->targetNodeId = static_cast<uint16_t>(targetNodeIdInBrick + nodeOffset);
            synapse->hardening = 0.0f;

            // make to blocking synapse by random
            if(rand() % 2 == 0) {
                synapse->sign = -1;
            }

            synapse->dynamicWeight = usedLearn;
            weight -= usedLearn;
            pos++;
        }
    }

    if(pos == SYNAPSES_PER_SYNAPSESECTION
            && section->next == UNINIT_STATE_32)
    {
        findNewSectioin(sectionPos);
    }

    if(weight > 2.0f) {
        synapseProcessing(section->next, weight, hardening);
    }
}

//==================================================================================================

inline void
updating(const uint32_t sectionPos)
{
    SynapseSection* synapseSections = getBuffer<SynapseSection>(KyoukoRoot::m_segment->synapses);
    SynapseSection* section = &synapseSections[sectionPos];

    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    Node* nodes = getBuffer<Node>(KyoukoRoot::m_segment->nodes);

    // iterate over all synapses in synapse-section
    uint32_t currentPos = 0;
    for(uint32_t lastPos = 0; lastPos < SYNAPSES_PER_SYNAPSESECTION; lastPos++)
    {
        Synapse* synapse = &section->synapses[lastPos];
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
            section->synapses[currentPos] = section->synapses[lastPos];
            currentPos++;
        }
    }

    if(section->next != UNINIT_STATE_32) {
        updating(section->next);
    }

    // delete if sections is empty
    if(currentPos == 0) {
        removeSection(sectionPos);
    }
}

//==================================================================================================

void
node_processing()
{
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    Node* nodes = getBuffer<Node>(KyoukoRoot::m_segment->nodes);
    const uint64_t numberOfNodes = KyoukoRoot::m_segment->nodes.numberOfItems;

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

    for(uint32_t i = 0; i < numberOfNodes / 256; i++)
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
                synapseProcessing(i, weight, globalValue->lerningValue);
            }
            else
            {
                node->active = 0;
                updating(i);
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
