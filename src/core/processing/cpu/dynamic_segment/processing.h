/**
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

#ifndef KYOUKOMIND_DYNAMIC_PROCESSING_H
#define KYOUKOMIND_DYNAMIC_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/data_structure/segments/dynamic_segment.h>
#include <core/objects/synapses.h>

#include <core/processing/cpu/dynamic_segment/create_reduce.h>

/**
 * @brief process synapse-section
 *
 * @param section current processed synapse-section
 * @param segment refernece to the processed segment
 * @param sourceNode source-node, who triggered the section
 * @param weightIn wight-value, which comes into the section
 * @param outH multiplicator
 */
inline void
synapseProcessing(SynapseSection* section,
                  DynamicSegment &segment,
                  DynamicNode* sourceNode,
                  const float weightIn,
                  const float outH)
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
                    && segment.dynamicSegmentSettings->doLearn > 0;
        // create new synapse if necesarry and learning is active
        if(createSyn)
        {
            createNewSynapse(section,
                             synapse,
                             segment.bricks,
                             sourceNode,
                             segment.dynamicSegmentSettings,
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
            const uint64_t newPos = createNewSection(segment);
            if(newPos == ITEM_BUFFER_UNDEFINE_POS) {
                return;
            }

            section->next = newPos;
        }

        synapseProcessing(&segment.synapseSections[section->next],
                          segment,
                          sourceNode,
                          netH,
                          outH);
    }
}

/**
 * @brief reset single node
 *
 * @param node pointer to node to reset
 */
inline
void initNode(DynamicNode* node)
{
    const bool initNode = node->isInit == false && node->input > 0.0f;
    node->isInit = node->isInit || initNode;
    node->border = static_cast<float>(initNode) * node->input * 0.5f
                   + static_cast<float>(initNode == false) * node->border;
}

/**
 * @brief reset nodes of a input brick
 *
 * @param brick pointer to the brick
 * @param segment segment where the brick belongs to
 */
inline void
prepareNodesOfInputBrick(Brick* brick,
                         DynamicSegment &segment)
{
    DynamicNode* node = nullptr;

    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];
        node->input = segment.inputTransfers[node->targetBorderId];
        node->potential = segment.dynamicSegmentSettings->potentialOverflow * node->input;
        initNode(node);
        node->input = 0.0f;
    }
}

/**
 * @brief reset nodes of a output brick
 *
 * @param brick pointer to the brick
 * @param segment segment where the brick belongs to
 */
inline void
prepareNodesOfOutputBrick(Brick* brick,
                          DynamicSegment &segment)
{
    DynamicNode* node = nullptr;

    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];
        node->potential = segment.dynamicSegmentSettings->potentialOverflow * node->input;
        node->potential = 1.0f / (1.0f + exp(-1.0f * node->potential));
        segment.outputTransfers[node->targetBorderId] = node->potential;
        initNode(node);
        node->input = 0.0f;
    }
}

/**
 * @brief reset nodes of a normal brick
 *
 * @param brick pointer to the brick
 * @param segment segment where the brick belongs to
 */
inline void
prepareNodesOfNormalBrick(Brick* brick,
                          DynamicSegment &segment)
{
    DynamicNode* node = nullptr;

    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];
        node->potential = segment.dynamicSegmentSettings->potentialOverflow * node->input;
        initNode(node);
        node->input = 0.0f;
    }
}

/**
 * @brief process all nodes within a specific brick and also all synapse-sections,
 *        which are connected to an active node
 *
 * @param brick brick, which should be processed
 * @param segment segment to process
 */
inline void
nodeProcessing(Brick* brick,
               DynamicSegment &segment)
{
    bool active = false;
    float outH = 0.0f;
    DynamicNode* node = nullptr;
    uint64_t newPos = 0;

    if(brick->isInputBrick)
    {
        prepareNodesOfInputBrick(brick, segment);
    }
    else if(brick->isOutputBrick)
    {
        prepareNodesOfOutputBrick(brick, segment);
        return;
    }
    else
    {
        prepareNodesOfNormalBrick(brick, segment);
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
                newPos = createNewSection(segment);
                if(newPos == ITEM_BUFFER_UNDEFINE_POS) {
                    continue;
                }

                node->targetSectionId = newPos;
            }

            outH = 1.0f / (1.0f + exp(-1.0f * node->potential));
            synapseProcessing(&segment.synapseSections[node->targetSectionId],
                              segment,
                              node,
                              node->potential,
                              outH);
        }

        node->active = active;
    }
}

/**
 * @brief process all nodes within a specific brick and also all synapse-sections,
 *        which are connected to an active node
 *
 * @param segment segment to process
 */
void
prcessDynamicSegment(DynamicSegment* segment)
{
    const uint32_t numberOfBricks = segment->segmentHeader->bricks.count;
    for(uint32_t pos = 0; pos < numberOfBricks; pos++)
    {
        const uint32_t brickId = segment->brickOrder[pos];
        Brick* brick = &segment->bricks[brickId];
        nodeProcessing(brick, *segment);
    }
}

#endif // KYOUKOMIND_DYNAMIC_PROCESSING_H
