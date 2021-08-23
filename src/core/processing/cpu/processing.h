﻿/**
 * @file        processing.h
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

#include <core/processing/cpu/create_reduce.h>

/**
 * @brief synapseProcessing
 *
 * @param section current processed synapse-section
 * @param bricks array of all bricks
 * @param nodes array of all nodes in the current brick
 * @param randomValues array of precreated random values for the learning process
 * @param segmentSettings settings of the section
 * @param sourceNode source-node, who triggered the section
 * @param weightIn wight-value, which comes into the section
 */
inline void
synapseProcessing(SynapseSection* section,
                  Segment &segment,
                  Node* sourceNode,
                  const float weightIn,
                  const float outH,
                  const uint32_t* randomValues)
{
    uint32_t pos = 0;
    float netH = weightIn;
    bool createSyn = false;
    Synapse* synapse = nullptr;
    section->updated = 0;

    // iterate over all synapses in the section
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && netH > 0.0f)
    {
        synapse = &section->synapses[pos];
        createSyn = synapse->targetNodeId == UNINIT_STATE_16
                    && pos >= section->hardening
                    && segment.segmentSettings->doLearn > 0;
        // create new synapse if necesarry and learning is active
        if(createSyn)
        {
            createNewSynapse(section,
                             synapse,
                             segment.bricks,
                             randomValues,
                             sourceNode,
                             segment.segmentSettings,
                             netH);
        }

        // break loop, if learning is disabled to the loop has reached an inactive synapse
        if(synapse->targetNodeId == UNINIT_STATE_16) {
            break;
        }

        // update synapse
        segment.nodes[synapse->targetNodeId].input += outH * synapse->weight;
        synapse->activeCounter += (synapse->activeCounter < 126);

        // update loop-counter
        netH -= static_cast<float>(synapse->border) * BORDER_STEP;
        pos++;
    }

    if(netH > 0.1f)
    {
        if(section->next == UNINIT_STATE_32)
        {
            SynapseSection newSection;
            newSection.active = Kitsunemimi::ItemBuffer::ACTIVE_SECTION;
            newSection.randomPos = rand() % NUMBER_OF_RAND_VALUES;
            newSection.brickBufferPos = randomValues[newSection.randomPos] % 1000;

            const uint64_t newPos = segment.segmentData.addNewItem(newSection);
            if(newPos == ITEM_BUFFER_UNDEFINE_POS) {
                return;
            }

            section->next = newPos;
        }

        synapseProcessing(&segment.synapseSections[section->next],
                          segment,
                          sourceNode,
                          netH,
                          outH,
                          randomValues);
    }
}

/**
 * @brief process all nodes within a specific brick and also all synapse-sections,
 *        which are connected to an active node
 *
 * @param brick brick, which should be processed
 * @param nodes array of all nodes in the brick
 * @param synapseSections array of all synapse-sections
 * @param bricks array of all bricks
 * @param randomValues array of precreated random values for the learning process
 * @param segmentSettings settings of the section
 */
inline void
nodeProcessing(Brick* brick,
               Segment &segment,
               const uint32_t* randomValues)
{
    bool initNode = false;
    bool active = false;
    Node* node = nullptr;

    // process all nodes within the brick
    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];
        initNode = node->init == 0
                   && node->input > 0.0f;
        node->border = static_cast<float>(initNode) * node->input * 0.5f
                       + static_cast<float>(initNode == false) * node->border;
        node->potential = segment.segmentSettings->potentialOverflow * node->input;
        node->input = 0.0f;
        node->delta = 0.0f;
    }

    // do not process output-bricks any further
    if(brick->isOutputBrick) {
        return;
    }

    // process all synapse-sections, which are connected to an active node within the brick
    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];
        active = node->potential > node->border;
        if(active)
        {
            if(node->targetSectionId == UNINIT_STATE_32)
            {
                SynapseSection newSection;
                newSection.active = Kitsunemimi::ItemBuffer::ACTIVE_SECTION;
                newSection.randomPos = rand() % NUMBER_OF_RAND_VALUES;
                newSection.brickBufferPos = randomValues[newSection.randomPos] % 1000;

                const uint64_t newPos = segment.segmentData.addNewItem(newSection);
                if(newPos == ITEM_BUFFER_UNDEFINE_POS) {
                    continue;
                }

                node->targetSectionId = newPos;
            }

            const float outH = 1.0f / (1.0f + exp(-1.0f * node->potential));
            synapseProcessing(&segment.synapseSections[node->targetSectionId],
                              segment,
                              node,
                              node->potential,
                              outH,
                              randomValues);
        }

        node->active = active;
    }
}

#endif // SYNAPSE_PROCESSING_H
