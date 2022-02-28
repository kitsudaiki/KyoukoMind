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

#ifndef KYOUKOMIND_STATIC_PROCESSING_H
#define KYOUKOMIND_STATIC_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/segments/brick.h>

#include "node.h"
#include "static_segment.h"

/**
 * @brief reset nodes of a input brick
 *
 * @param brick pointer to the brick
 * @param segment segment where the brick belongs to
 */
inline void
processInputNodes(Brick* brick,
                  StaticSegment &segment)
{
    StaticNode* node = nullptr;

    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];
        node->delta = 0.0f;
        node->value = segment.inputTransfers[node->targetBorderId];
    }
}

/**
 * @brief reset nodes of a output brick
 *
 * @param brick pointer to the brick
 * @param segment segment where the brick belongs to
 */
inline void
processOutputNodes(Brick* brick,
                   StaticSegment &segment)
{
    StaticNode* node = nullptr;

    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];
        node->delta = 0.0f;
        segment.outputTransfers[node->targetBorderId] = node->value;
    }
}

/**
 * @brief reset nodes of a normal brick
 *
 * @param brick pointer to the brick
 * @param segment segment where the brick belongs to
 */
inline void
processNormalNodes(Brick* brick,
                   Brick* lastBrick,
                   StaticSegment &segment)
{
    StaticNode* node = nullptr;
    StaticNode* lastNodes = &segment.nodes[lastBrick->nodePos];
    float* connection = nullptr;
    float weight = 0.0f;

    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];

        node->delta = 0.0f;
        connection = &segment.connections[node->targetConnectionPos];
        weight = 0.0f;

        for(uint32_t i = 0; i < lastBrick->numberOfNodes; i++) {
            weight += (lastNodes[i].value * connection[i]) + node->border;
        }

        node->value = 1.0f / (1.0f + exp(-1.0f * weight));
    }
}

/**
 * @brief process all nodes within a specific brick and also all synapse-sections,
 *        which are connected to an active node
 *
 * @param segment segment to process
 */
void
processStaticSegment(StaticSegment* segment)
{
    const uint32_t numberOfBricks = segment->segmentHeader->bricks.count;
    for(uint32_t pos = 0; pos < numberOfBricks; pos++)
    {
        Brick* brick = &segment->bricks[pos];
        if(brick->isInputBrick)
        {
            processInputNodes(brick, *segment);
        }
        else
        {
            processNormalNodes(brick, &segment->bricks[pos-1], *segment);
            if(brick->isOutputBrick) {
                processOutputNodes(brick, *segment);
            }
        }
    }
}

#endif // KYOUKOMIND_STATIC_PROCESSING_H
