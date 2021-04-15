/**
 * @file        synapse_processing.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#ifndef SYNAPSE_PROCESSING_H
#define SYNAPSE_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/synapses.h>
#include <core/objects/global_values.h>

#include <libKitsunemimiCommon/buffer/item_buffer.h>


/**
 * @brief synapseProcessing
 * @param sectionPos
 * @param weight
 * @param hardening
 */
inline void
synapseProcessing(SynapseSection* section,
                  const uint32_t nodeId,
                  const float weightIn,
                  const uint32_t layer)

{
    Segment* seg = KyoukoRoot::m_segment;
    float* nodeBuffer = Kitsunemimi::getBuffer<float>(seg->nodeProcessingBuffer);
    const uint64_t numberOfNodes = seg->nodes.numberOfItems;
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(seg->globalValues);
    const uint64_t numberOfSynapses = seg->synapses.numberOfItems;
    SynapseBuffer* synapseBuffers = Kitsunemimi::getBuffer<SynapseBuffer>(seg->synapseBuffer);
    Node* nodes = Kitsunemimi::getBuffer<Node>(seg->nodes);

    uint32_t pos = 0;
    uint32_t counter = 0;
    float weight = weightIn;
    bool processed = false;
    const float maxWeight = globalValue->maxSynapseWeight;
    Node* node = &nodes[nodeId];

    // reinit section if necessary
    if(section->active == 0)
    {
        section->active = 1;
        section->brickBufferPos = rand() & 1000;
    }

    // iterate over all synapses in the section and update the target-nodes
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && weight > 0.0f)
    {
        Synapse* synapse = &section->synapses[pos];

        // create new synapse
        if(synapse->targetNodeId == UNINIT_STATE_16
                && pos >= section->hardening
                && globalValue->doLearn > 0
                && section->next == UNINIT_STATE_64)
        {
            // set new weight
            const float random = (rand() % 1024) / 1024.0f;
            const float tooLearn = maxWeight * random;
            synapse->weight = static_cast<float>(weight < tooLearn) * weight
                                    + static_cast<float>(weight >= tooLearn) * tooLearn;

            // get random node-id as target
            const uint32_t targetNodeIdInBrick = static_cast<uint32_t>(rand())
                                                 % globalValue->nodesPerBrick;
            Brick* nodeBrick = seg->nodeBricks[node->nodeBrickId];
            const uint32_t nodeOffset = nodeBrick->possibleTargetNodeBrickIds[section->brickBufferPos]
                                        * globalValue->nodesPerBrick;
            synapse->targetNodeId = static_cast<uint16_t>(targetNodeIdInBrick + nodeOffset);

            // set sign
            const uint32_t signRand = rand() % 1000;
            const float signNeg = globalValue->signNeg;
            synapse->sign = 1 - (1000.0f * signNeg > signRand) * 2;
        }

        pos++;

        // process synapse
        if(synapse->targetNodeId != UNINIT_STATE_16)
        {
            // 0 because only one thread at the moment
            const ulong nodeBufferPosition = (0 * numberOfNodes) + synapse->targetNodeId;
            const float synapseWeight = synapse->weight;
            const float shareWeight = static_cast<float>(weight > synapseWeight) * synapseWeight
                                      + static_cast<float>(weight <= synapseWeight) * weight;

            nodeBuffer[nodeBufferPosition] += (shareWeight * static_cast<float>(synapse->sign));

            weight -= shareWeight;
            counter = pos;
            processed = true;
        }
    }

    // harden synapse-section
    if(globalValue->lerningValue > 0.0f)
    {
        if(counter > section->hardening) {
            section->hardening = counter;
        }
    }

    // go to next section
    if(weight > 1.0f
            && processed)
    {
        uint32_t nextLayer = layer + 1;
        nextLayer = (nextLayer > 7) * 7  + (nextLayer <= 7) * nextLayer;
        const uint32_t pos = (node->targetSectionId + nextLayer * 10000 + nextLayer) % numberOfSynapses;
        SynapseBuffer* synapseBuffer = &synapseBuffers[pos];
        synapseBuffer->buffer[nextLayer].weigth = weight;
        synapseBuffer->buffer[nextLayer].nodeId = nodeId;
        synapseBuffer->process = 1;
    }
}

/**
 * @brief updating
 * @param sectionPos
 */
inline bool
updating(SynapseSection* section)
{
    bool upToData = 1;

    Segment* seg = KyoukoRoot::m_segment;
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(seg->globalValues);
    Node* nodes = Kitsunemimi::getBuffer<Node>(seg->nodes);

    // iterate over all synapses in synapse-section
    uint32_t currentPos = section->hardening;
    for(uint32_t lastPos = section->hardening; lastPos < SYNAPSES_PER_SYNAPSESECTION; lastPos++)
    {
        Synapse* synapse = &section->synapses[lastPos];

        if(synapse->targetNodeId == UNINIT_STATE_16) {
            continue;
        }

        upToData = 0;

        // update dynamic-weight-value of the synapse
        if(nodes[synapse->targetNodeId].active == 0) {
            synapse->weight = synapse->weight * 0.0f;
        } else {
            synapse->weight = synapse->weight * 0.0f;
        }

        // check for deletion of the single synapse
        if(synapse->weight < globalValue->synapseDeleteBorder)
        {
            synapse->weight = 0.0f;
            synapse->targetNodeId = UNINIT_STATE_16;
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

    // delete if sections is empty
    if(section->hardening == 0
            && currentPos == 0)
    {
        section->active = 0;
        upToData = 1;
    }

    return upToData;
}

/**
 * @brief synapse_processing
 */
inline void
synapse_processing()
{
    Segment* seg = KyoukoRoot::m_segment;
    SynapseSection* sections = Kitsunemimi::getBuffer<SynapseSection>(seg->synapses);
    SynapseBuffer* synapseBuffers = Kitsunemimi::getBuffer<SynapseBuffer>(seg->synapseBuffer);

    const uint64_t numberOfSynapses = seg->synapses.numberOfItems;

    //----------------------------------------------------------------------------------------------
    for(uint32_t i = 0; i < numberOfSynapses; i++)
    {
        SynapseBuffer* synapseBuffer = &synapseBuffers[i];

        if(synapseBuffer->process == 0)
        {
            if(synapseBuffer->upToDate == 0) {
                synapseBuffer->upToDate = updating(&sections[i]);
            }
            continue;
        }

        for(uint8_t layer = 0; layer < 8; layer++)
        {
            SynapseBufferEntry* entry = &synapseBuffer->buffer[layer];

            if(entry->weigth > 5.0f)
            {
                synapseProcessing(&sections[i], entry->nodeId, entry->weigth, layer);
                synapseBuffer->upToDate = 0;
            }

            entry->weigth = 0.0f;
            entry->nodeId = UNINIT_STATE_32;
        }
    }
}

/**
 * @brief node_processing
 */
inline void
node_processing()
{
    Segment* seg = KyoukoRoot::m_segment;
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(seg->globalValues);
    Node* nodes = Kitsunemimi::getBuffer<Node>(seg->nodes);
    float* inputNodes = Kitsunemimi::getBuffer<float>(seg->nodeInputBuffer);
    float* nodeProcessingBuffer = Kitsunemimi::getBuffer<float>(seg->nodeProcessingBuffer);
    float* transferNodes = Kitsunemimi::getBuffer<float>(seg->transferNodeBuffer);
    SynapseBuffer* synapseBuffer = Kitsunemimi::getBuffer<SynapseBuffer>(seg->synapseBuffer);

    const uint64_t numberOfNodes = seg->nodes.numberOfItems;

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
            // check if active
            const bool reset = node->border < node->currentState
                               && node->refractionTime == 0;
            if(reset)
            {
                node->potential = globalValue->actionPotential +
                                  globalValue->potentialOverflow * node->currentState;
                node->refractionTime = globalValue->refractionTime;
            }

            // set to 255.0f, if value is too high
            const float cur = node->currentState;
            node->currentState = static_cast<float>(cur < 0.0f) * 0.0f
                                 + static_cast<float>(cur >= 0.0f) * cur;

            synapseBuffer[i].buffer[0].weigth = node->potential;
            synapseBuffer[i].buffer[0].nodeId = i;
            synapseBuffer[i].process = node->potential > 5.0f;

            // post-steps
            node->refractionTime = node->refractionTime >> 1;

            // set to 0.0f, if value is negative
            const float newCur = node->currentState;
            node->currentState = static_cast<float>(newCur < 0.0f) * 0.0f
                                 + static_cast<float>(newCur >= 0.0f) * newCur;

            // make cooldown in the node
            node->potential /= globalValue->nodeCooldown;
            node->currentState /= globalValue->nodeCooldown;
        }
        else if(node->border == 0.0f)
        {
            node->potential = inputNodes[i];
            synapseBuffer[i].buffer[0].weigth = node->potential;
            synapseBuffer[i].buffer[0].nodeId = i;
            synapseBuffer[i].process = node->potential > 5.0f;
        }
        else
        {
            const float newCur = node->currentState;
            node->currentState = static_cast<float>(newCur < 0.0f) * 0.0f
                                 + static_cast<float>(newCur >= 0.0f) * newCur;
            const float pot = globalValue->potentialOverflow * node->currentState;
            transferNodes[i % globalValue->nodesPerBrick] = pot;
            node->currentState = 0.0f;
        }
    }
}

#endif // SYNAPSE_PROCESSING_H
