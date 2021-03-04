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

#include "section_handling.h"

#include <libKitsunemimiCommon/buffer/item_buffer.h>


/**
 * @brief synapseProcessing
 * @param sectionPos
 * @param weight
 * @param hardening
 */
inline void
synapseProcessing(const uint64_t sectionPos,
                  const float weightIn,
                  const uint32_t sourceNodeBrickId,
                  const float maxSteps,
                  float currentStep,
                  const float gradiant)
{
    Segment* seg = KyoukoRoot::m_segment;
    SynapseSection* synapseSections = Kitsunemimi::getBuffer<SynapseSection>(seg->synapses);
    SynapseSection* section = &synapseSections[sectionPos];
    float* nodeBuffer = Kitsunemimi::getBuffer<float>(seg->nodeProcessingBuffer);
    const uint64_t numberOfNodes = seg->nodes.numberOfItems;
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(seg->globalValues);

    uint32_t pos = 0;
    uint32_t counter = 0;
    float weight = weightIn;
    const float maxWeight = globalValue->maxSynapseWeight;

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
            if(pos % 2 == 0)
            {
                const float random = (rand() % 1024) / 1024.0f;
                const float usedLearn = (weight < 2.0f) * weight
                                        + (weight >= 2.0f) * ((weight * random) + 1.0f);
                synapse->weight = fmod(usedLearn, maxWeight);
            }
            else
            {
                synapse->weight = maxWeight - section->synapses[pos - 1].weight;
            }

            // get random node-id as target
            const uint32_t targetNodeIdInBrick = static_cast<uint32_t>(rand())
                                                 % globalValue->nodesPerBrick;
            const uint32_t nodeOffset = section->nodeBrickId * globalValue->nodesPerBrick;
            synapse->targetNodeId = static_cast<uint16_t>(targetNodeIdInBrick + nodeOffset);
            synapse->sign = 1 - (rand() % 2) * 2;
        }

        pos++;

        // process synapse
        if(synapse->targetNodeId != UNINIT_STATE_16)
        {
            // 0 because only one thread at the moment
            const ulong nodeBufferPosition = (0 * numberOfNodes) + synapse->targetNodeId;
            const float synapseWeight = synapse->weight;
            const float shareWeight = (weight > synapseWeight) * synapseWeight
                                      + (weight <= synapseWeight) * weight;
            currentStep += 1.0f;
            const float additionalWeight = (currentStep / maxSteps) * maxWeight * gradiant;

            nodeBuffer[nodeBufferPosition] += (shareWeight * static_cast<float>(synapse->sign))
                                              + additionalWeight;

            weight -= shareWeight;
            counter = pos;
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
    if(weight > 1.0f)
    {
        // create new section if necessary
        if(globalValue->doLearn > 0
                && section->next == UNINIT_STATE_64)
        {
            findNewSectioin(synapseSections,
                            sectionPos,
                            sourceNodeBrickId);
        }

        // process next section
        if(section->next != UNINIT_STATE_64)
        {
            synapseProcessing(section->next,
                              weight,
                              sourceNodeBrickId,
                              maxSteps,
                              currentStep,
                              gradiant);
        }
    }
}

/**
 * @brief updating
 * @param sectionPos
 */
inline void
updating(const uint64_t sectionPos)
{
    Segment* seg = KyoukoRoot::m_segment;
    SynapseSection* synapseSections = Kitsunemimi::getBuffer<SynapseSection>(seg->synapses);
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(seg->globalValues);
    Node* nodes = Kitsunemimi::getBuffer<Node>(seg->nodes);
    SynapseSection* section = &synapseSections[sectionPos];

    // update next-section
    if(section->next != UNINIT_STATE_64) {
        updating(section->next);
    }

    // iterate over all synapses in synapse-section
    uint32_t currentPos = section->hardening;
    for(uint32_t lastPos = section->hardening; lastPos < SYNAPSES_PER_SYNAPSESECTION; lastPos++)
    {
        Synapse* synapse = &section->synapses[lastPos];

        if(synapse->targetNodeId == UNINIT_STATE_16) {
            continue;
        }

        // update dynamic-weight-value of the synapse
        if(nodes[synapse->targetNodeId].active == 0) {
            synapse->weight = synapse->weight * globalValue->memorizing;
        } else {
            synapse->weight = synapse->weight * 0.95f;
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
            && section->prev != UNINIT_STATE_64
            && currentPos == 0)
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
triggerSynapseSesction(Brick* brick,
                       Node* node,
                       const uint32_t i,
                       GlobalValues* globalValue,
                       const float gradiant)
{
    if(node->potential > 10.0f)
    {
        node->active = 1;
        // build new axon-transfer-edge, which is send back to the host
        const float up = static_cast<float>(pow(globalValue->gliaValue, node->targetBrickDistance));
        const float weight = node->potential * up;
        brick->nodeActivity++;
        const float maxSteps = weight / globalValue->maxSynapseWeight;
        synapseProcessing(i, weight, brick->nodeBrickId, maxSteps, 0.0f, gradiant);
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
    Segment* seg = KyoukoRoot::m_segment;
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(seg->globalValues);
    Node* nodes = Kitsunemimi::getBuffer<Node>(seg->nodes);
    float* inputNodes = Kitsunemimi::getBuffer<float>(seg->nodeInputBuffer);
    float* nodeProcessingBuffer = Kitsunemimi::getBuffer<float>(seg->nodeProcessingBuffer);
    float* outputNodes = Kitsunemimi::getBuffer<float>(seg->nodeOutputBuffer);
    Brick** nodeBricks = seg->nodeBricks;

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

            triggerSynapseSesction(nodeBricks[node->nodeBrickId], node, i, globalValue, 0.0001f);

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
            triggerSynapseSesction(nodeBricks[node->nodeBrickId], node, i, globalValue, 0.1f);
        }
        else
        {
            nodeBricks[node->nodeBrickId]->nodeActivity++;
            outputNodes[i % globalValue->nodesPerBrick] = node->currentState;
            node->currentState = 0.0f;
        }
    }
}

#endif // SYNAPSE_PROCESSING_H
