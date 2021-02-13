#ifndef SYNAPSE_PROCESSING_H
#define SYNAPSE_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/item_buffer.h>
#include <core/objects/synapses.h>
#include <core/objects/global_values.h>

#include "output_synapse_processing.h"

/**
 * @brief findNewSectioin
 * @param synapseSections
 * @param oldSectionId
 * @return
 */
inline bool
findNewSectioin(SynapseSection* synapseSections, const uint32_t oldSectionId)
{
    const uint64_t numberOfSections = KyoukoRoot::m_segment->synapses.itemCapacity;
    Brick** nodeBricks = KyoukoRoot::m_segment->nodeBricks;

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

            const uint32_t nodeBrickPos = rand() % KyoukoRoot::m_segment->numberOfNodeBricks;
            synapseSections->nodeBrickId = nodeBricks[nodeBrickPos]->nodeBrickId;

            std::cout<<"create"<<std::endl;
            return true;
        }
    }

    return false;
}

/**
 * @brief removeSection
 * @param synapseSections
 * @param pos
 */
inline void
removeSection(SynapseSection* synapseSections, const uint32_t pos)
{
    SynapseSection* section = &synapseSections[pos];
    SynapseSection* prev = &synapseSections[section->prev];

    if(section->next != UNINIT_STATE_32)
    {
        SynapseSection* next = &synapseSections[section->next];
        next->prev = section->prev;
    }

    prev->next = section->next;

    SynapseSection emptyEdge;
    synapseSections[pos] = emptyEdge;

    std::cout<<"delete"<<std::endl;
}

/**
 * @brief synapseProcessing
 * @param sectionPos
 * @param weight
 * @param hardening
 */
inline void
synapseProcessing(const uint32_t sectionPos,
                  float weight)
{
    SynapseSection* synapseSections = getBuffer<SynapseSection>(KyoukoRoot::m_segment->synapses);
    SynapseSection* section = &synapseSections[sectionPos];
    float* nodeProcessingBuffer = getBuffer<float>(KyoukoRoot::m_segment->nodeProcessingBuffer);

    const uint64_t numberOfNodes = KyoukoRoot::m_segment->nodes.numberOfItems;
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    uint32_t pos = 0;

    // iterate over all synapses in the section and update the target-nodes
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && weight > 2.0f)
    {
        Synapse* synapse = &section->synapses[pos];
        if(synapse->targetNodeId == UNINIT_STATE_16)
        {
            synapse->hardening = 0.0f;

            // set new weight
            const float random = (rand() % 1024) / 1024.0f;
            const float usedLearn = (weight < 5.0f) * weight
                                    + (weight >= 5.0f) * ((weight * random) + 1.0f);
            synapse->dynamicWeight = usedLearn;

            // get random node-id as target
            const uint32_t targetNodeIdInBrick = static_cast<uint32_t>(rand())
                                                 % globalValue->numberOfNodesPerBrick;
            const uint32_t nodeOffset = section->nodeBrickId * globalValue->numberOfNodesPerBrick;
            synapse->targetNodeId = static_cast<uint16_t>(targetNodeIdInBrick + nodeOffset);

            // sign be random 1 or -1
            synapse->sign = 1 - (rand() % 2) * 2;
        }

        float newHardening = synapse->hardening + globalValue->lerningValue;
        newHardening = (newHardening > 1.0f) * 1.0f + (newHardening <= 1.0f) * newHardening;

        // update static weight value
        const float hardeningDiff = newHardening - synapse->hardening;
        const float diff = synapse->dynamicWeight * hardeningDiff;
        synapse->dynamicWeight -= diff;
        synapse->staticWeight += diff;
        synapse->hardening = newHardening;

        // 0 because only one thread at the moment
        const ulong nodeBufferPosition = (0 * numberOfNodes) + synapse->targetNodeId;
        const float synapseWeight = synapse->staticWeight + synapse->dynamicWeight;
        const float shareWeight = (weight > synapseWeight) * synapseWeight
                                  + (weight <= synapseWeight) * weight;

        nodeProcessingBuffer[nodeBufferPosition] += shareWeight * static_cast<float>(synapse->sign);

        weight -= shareWeight;
        pos++;
    }

    if(pos == SYNAPSES_PER_SYNAPSESECTION
            && section->next == UNINIT_STATE_32)
    {
        findNewSectioin(synapseSections, sectionPos);
    }

    if(weight > 2.0f) {
        synapseProcessing(section->next, weight);
    }
}

/**
 * @brief updating
 * @param sectionPos
 */
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
            synapse->hardening = 0.0f;
            synapse->sign = 1;
        }
        else
        {
            const Synapse currentSyn = section->synapses[currentPos];
            section->synapses[currentPos] = section->synapses[lastPos];
            section->synapses[lastPos] = currentSyn;
            currentPos++;
        }
    }

    if(section->next != UNINIT_STATE_32) {
        updating(section->next);
    }

    // delete if sections is empty
    if(currentPos == 0
            && section->prev != UNINIT_STATE_32)
    {
        removeSection(synapseSections, sectionPos);
    }
}

/**
 * @brief triggerSynapseSesction
 * @param node
 * @param i
 * @param globalValue
 */
inline void
triggerSynapseSesction(Node* node,
                       const uint32_t i,
                       GlobalValues* globalValue)
{
    if(node->potential > 10.0f)
    {
        node->active = 1;
        // build new axon-transfer-edge, which is send back to the host
        const float up = static_cast<float>(pow(globalValue->gliaValue, node->targetBrickDistance));
        const float weight = node->potential * up;
        synapseProcessing(i, weight);
    }
    else
    {
        node->active = 0;
        updating(i);
    }
}

/**
 * @brief node_processing
 */
void
node_processing()
{
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    Node* nodes = getBuffer<Node>(KyoukoRoot::m_segment->nodes);
    float* inputNodes = getBuffer<float>(KyoukoRoot::m_segment->nodeInputBuffer);
    float* nodeProcessingBuffer = getBuffer<float>(KyoukoRoot::m_segment->nodeProcessingBuffer);

    const uint64_t numberOfNodes = KyoukoRoot::m_segment->nodes.numberOfItems;

    for(uint64_t i = 0; i < numberOfNodes; i++)
    {
        // TODO: when port to gpu: change 2 to 256 again
        for(uint pos = 0; pos < 1; pos++)
        {
            const ulong nodeBufferPosition = (pos * (numberOfNodes)) + i;
            nodes[i].currentState += nodeProcessingBuffer[nodeBufferPosition];
            nodeProcessingBuffer[nodeBufferPosition] = 0.0f;
        }
    }

    for(uint32_t i = 0; i < numberOfNodes; i++)
    {
        Node* node = &nodes[i];
        if(node->border > 0.0f)
        {
            // set to 255.0f, if value is too high
            const float cur = node->currentState;
            node->currentState = (cur > 255.0f) * 255.0f + (cur <= 255.0f) * cur;
            node->currentState = (cur < 0.0f) * 0.0f + (cur >= 0.0f) * cur;

            // check if active
            const bool reset = node->border < node->currentState
                               && node->refractionTime == 0;
            node->potential = reset * globalValue->actionPotential
                              + (reset == false) * node->potential;
            node->refractionTime = reset * globalValue->refractionTime
                                   + (reset == false) * node->refractionTime;

            triggerSynapseSesction(node, i, globalValue);

            // post-steps
            node->refractionTime = node->refractionTime >> 1;

            // set to 0.0f, if value is negative
            const float newCur = node->currentState;
            node->currentState = (newCur < 0.0f) * 0.0f + (newCur >= 0.0f) * newCur;

            // make cooldown in the node
            node->potential /= globalValue->nodeCooldown;
            node->currentState /= globalValue->nodeCooldown;
        }
        else if(node->border == 0.0f)
        {
            node->potential = inputNodes[i];
            triggerSynapseSesction(node, i, globalValue);
        }
        else
        {
            node->potential = node->currentState;
            const uint32_t pos = i % globalValue->numberOfNodesPerBrick;
            outputSynapseProcessing(pos, node->potential);
        }
    }
}

#endif // SYNAPSE_PROCESSING_H
