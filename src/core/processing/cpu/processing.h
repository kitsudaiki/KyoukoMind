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

#include <libKitsunemimiAiCommon/metadata.h>

#include <core/processing/cpu/create_reduce.h>

/**
 * @brief synapseProcessing
 * @param section
 * @param bricks
 * @param nodes
 * @param randomValues
 * @param synapseMetaData
 * @param networkMetaData
 * @param sourceNode
 * @param weightIn
 */
inline void
synapseProcessing(SynapseSection* section,
                  Brick* bricks,
                  Node* nodes,
                  uint32_t* randomValues,
                  Kitsunemimi::Ai::SegmentSettings* synapseMetaData,
                  Node* sourceNode,
                  const float weightIn)
{
    // reinit section if necessary
    if(section->active == 0)
    {
        section->active = 1;
        section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
        section->brickBufferPos = randomValues[section->randomPos] % 1000;
    }

    uint32_t pos = 0;
    float netH = weightIn;
    const float outH = 1.0f / (1.0f + exp(-1.0f * netH));

    // iterate over all synapses in the section and update the target-nodes
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && netH > 0.0f)
    {
        Synapse* synapse = &section->synapses[pos];
        const bool createSyn = synapse->targetNodeId == UNINIT_STATE_16
                               && pos >= section->hardening
                               && synapseMetaData->doLearn > 0;
        if(createSyn)
        {
            createNewSynapse(section,
                             synapse,
                             bricks,
                             randomValues,
                             sourceNode,
                             synapseMetaData,
                             netH);
        }

        if(synapse->targetNodeId == UNINIT_STATE_16) {
            break;
        }

        // process synapse
        netH -= static_cast<float>(synapse->border) * BORDER_STEP;
        nodes[synapse->targetNodeId].input += outH * synapse->weight;
        synapse->activeCounter += (synapse->activeCounter < 126);

        pos++;
    }
}

/**
 * @brief nodeProcessing
 * @param brick
 * @param nodes
 * @param synapseSections
 * @param bricks
 * @param randomValues
 * @param synapseMetaData
 * @param networkMetaData
 */
inline void
nodeProcessing(Brick* brick,
               Node* nodes,
               SynapseSection* synapseSections,
               Brick* bricks,
               uint32_t* randomValues,
               Kitsunemimi::Ai::SegmentSettings* synapseMetaData)
{
    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        Node* node = &nodes[nodeId];
        const bool initNode = node->init == 0
                              && node->input > 0.0f;
        node->border = static_cast<float>(initNode) * node->input * 0.5f
                       + static_cast<float>(initNode == false) * node->border;
        node->potential = synapseMetaData->potentialOverflow * node->input;
        node->input = 0.0f;
        node->delta = 0.0f;
    }

    if(brick->isOutputBrick) {
        return;
    }

    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        Node* node = &nodes[nodeId];
        const bool active = node->potential > node->border;
        if(active)
        {
            synapseProcessing(&synapseSections[nodeId],
                              bricks,
                              nodes,
                              randomValues,
                              synapseMetaData,
                              node,
                              node->potential);
        }

        node->active = active;

    }
}

#endif // SYNAPSE_PROCESSING_H
