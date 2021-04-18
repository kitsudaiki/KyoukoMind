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
#include <core/objects/network_cluster.h>

/**
 * @brief synapseProcessing
 * @param sectionPos
 * @param weight
 * @param hardening
 */
inline void
synapseProcessing(SynapseSegment* segment,
                  SynapseSection* section,
                  Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
                  const uint32_t nodeId,
                  const float weightIn,
                  const uint32_t layer)

{
    uint32_t pos = 0;
    uint32_t counter = 0;
    float weight = weightIn;
    bool processed = false;
    const float maxWeight = segment->synapseMetaData->maxSynapseWeight;
    Node* node = &segment->nodes[nodeId];

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
                && networkMetaData->doLearn > 0
                && section->next == UNINIT_STATE_64)
        {
            // set new weight
            const float random = (rand() % 1024) / 1024.0f;
            const float tooLearn = maxWeight * random;
            synapse->weight = static_cast<float>(weight < tooLearn) * weight
                                    + static_cast<float>(weight >= tooLearn) * tooLearn;

            // get random node-id as target
            const uint32_t targetNodeIdInBrick = static_cast<uint32_t>(rand())
                                                 % segment->segmentMeta->numberOfNodesPerBrick;
            Brick* nodeBrick = &segment->nodeBricks[node->nodeBrickId];
            const uint32_t nodeOffset = nodeBrick->possibleTargetNodeBrickIds[section->brickBufferPos]
                                        * segment->segmentMeta->numberOfNodesPerBrick;
            synapse->targetNodeId = static_cast<uint16_t>(targetNodeIdInBrick + nodeOffset);

            // set sign
            const uint32_t signRand = rand() % 1000;
            const float signNeg = segment->synapseMetaData->signNeg;
            synapse->sign = 1 - (1000.0f * signNeg > signRand) * 2;

            synapse->multiplicator = (rand() % 5) + 1;
        }

        pos++;

        // process synapse
        if(synapse->targetNodeId != UNINIT_STATE_16)
        {
            // 0 because only one thread at the moment
            const ulong nodeBufferPosition = (0 * segment->segmentMeta->numberOfNodes) + synapse->targetNodeId;
            const float synapseWeight = synapse->weight;
            const float shareWeight = static_cast<float>(weight > synapseWeight) * synapseWeight
                                      + static_cast<float>(weight <= synapseWeight) * weight;

            segment->nodeBuffers[nodeBufferPosition] += (shareWeight * static_cast<float>(synapse->sign) * static_cast<float>(synapse->multiplicator));

            weight -= shareWeight;
            counter = pos;
            processed = true;
        }
    }

    // harden synapse-section
    if(networkMetaData->lerningValue > 0.0f)
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
        const uint32_t pos = (node->targetSectionId + nextLayer * 10000 + nextLayer) % segment->segmentMeta->numberOfSynapseSections;
        SynapseBuffer* synapseBuffer = &segment->synapseBuffers[pos];
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
updating(SynapseSegment* segment,
         SynapseSection* section)
{
    bool upToData = 1;

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
        if(segment->nodes[synapse->targetNodeId].active == 0) {
            synapse->weight = synapse->weight * 0.0f;
        } else {
            synapse->weight = synapse->weight * 0.0f;
        }

        // check for deletion of the single synapse
        if(synapse->weight < segment->synapseMetaData->synapseDeleteBorder)
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
synapse_processing(SynapseSegment* segment,
                   Kitsunemimi::Ai::NetworkMetaData* networkMetaData)
{
    const uint64_t numberOfSynapses = segment->segmentMeta->numberOfSynapseSections;

    //----------------------------------------------------------------------------------------------
    for(uint32_t i = 0; i < numberOfSynapses; i++)
    {
        SynapseBuffer* synapseBuffer = &segment->synapseBuffers[i];

        if(synapseBuffer->process == 0)
        {
            if(synapseBuffer->upToDate == 0) {
                synapseBuffer->upToDate = updating(segment, &segment->synapseSections[i]);
            }
            continue;
        }

        for(uint8_t layer = 0; layer < 8; layer++)
        {
            SynapseBufferEntry* entry = &synapseBuffer->buffer[layer];

            if(entry->weigth > 5.0f)
            {
                synapseProcessing(segment,
                                  &segment->synapseSections[i],
                                  networkMetaData,
                                  entry->nodeId,
                                  entry->weigth,
                                  layer);
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
node_processing(SynapseSegment* segment,
                OutputSegment* outputSegment)
{
    for(uint64_t i = 0; i < segment->segmentMeta->numberOfNodes; i++)
    {
        // TODO: when port to gpu: change 2 to 256 again
        for(uint pos = 0; pos < 1; pos++)
        {
            const ulong nodeBufferPosition = (pos * (segment->segmentMeta->numberOfNodes)) + i;
            segment->nodes[i].currentState += segment->nodeBuffers[nodeBufferPosition];
            segment->nodeBuffers[nodeBufferPosition] = 0.0f;
        }
    }

    for(uint64_t i = 0; i < segment->segmentMeta->numberOfInputs; i++) {
        segment->nodes[segment->inputNodes[i].targetNode].potential = segment->inputNodes[i].weight;
    }

    for(uint32_t i = 0; i < segment->segmentMeta->numberOfNodes; i++)
    {
        Node* node = &segment->nodes[i];
        if(node->border > 0.0f)
        {
            // check if active
            const bool reset = node->border < node->currentState
                               && node->refractionTime == 0;
            if(reset)
            {
                node->potential = segment->synapseMetaData->actionPotential +
                                  segment->synapseMetaData->potentialOverflow * node->currentState;
                node->refractionTime = segment->synapseMetaData->refractionTime;
            }

            // set to 255.0f, if value is too high
            const float cur = node->currentState;
            node->currentState = static_cast<float>(cur < 0.0f) * 0.0f + static_cast<float>(cur >= 0.0f) * cur;

            segment->synapseBuffers[i].buffer[0].weigth = node->potential;
            segment->synapseBuffers[i].buffer[0].nodeId = i;
            segment->synapseBuffers[i].process = node->potential > 5.0f;

            // post-steps
            node->refractionTime = node->refractionTime >> 1;

            // set to 0.0f, if value is negative
            const float newCur = node->currentState;
            node->currentState = static_cast<float>(newCur < 0.0f) * 0.0f + static_cast<float>(newCur >= 0.0f) * newCur;

            // make cooldown in the node
            node->potential /= segment->synapseMetaData->nodeCooldown;
            node->currentState /= segment->synapseMetaData->nodeCooldown;
        }
        else if(node->border == 0.0f)
        {
            segment->synapseBuffers[i].buffer[0].weigth = node->potential;
            segment->synapseBuffers[i].buffer[0].nodeId = i;
            segment->synapseBuffers[i].process = node->potential > 5.0f;
        }
        else
        {
            const float newCur = node->currentState;
            node->currentState = static_cast<float>(newCur < 0.0f) * 0.0f + static_cast<float>(newCur >= 0.0f) * newCur;
            const float pot = segment->synapseMetaData->potentialOverflow * node->currentState;
            outputSegment->inputs[i % segment->segmentMeta->numberOfNodesPerBrick] = pot;
            node->currentState = 0.0f;
        }
    }
}

#endif // SYNAPSE_PROCESSING_H
