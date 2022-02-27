/**
 * @file        backpropagation.h
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

#ifndef KYOUKOMIND_STATIC_BACKPROPAGATION_H
#define KYOUKOMIND_STATIC_BACKPROPAGATION_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/data_structure/segments/static_segment.h>
#include <core/objects/synapses.h>

/**
 * @brief backpropagate values of an output-brick
 *
 * @param brick brick to process
 * @param segment segment where the brick belongs to
 */
inline void
backpropagateOutput(Brick* brick,
                    StaticSegment* segment)
{
    StaticNode* node = nullptr;

    // iterate over all nodes within the brick
    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        node = &segment->nodes[nodeId];
        node->delta = segment->inputTransfers[node->targetBorderId];
    }
}

/**
 * @brief backpropagateInput
 * @param brick
 * @param segment
 */
inline void
backpropagateInput(Brick* brick,
                   StaticSegment* segment)
{
    StaticNode* node = nullptr;

    // iterate over all nodes within the brick
    for(uint32_t i = brick->nodePos;
        i < brick->numberOfNodes + brick->nodePos;
        i++)
    {
        node = &segment->nodes[i];
        segment->outputTransfers[node->targetBorderId] = node->delta;
    }
}

/**
 * @brief run back-propagation over the hidden neurons
 *
 * @param brick pointer to current brick
 * @param segment pointer to currect segment to process, which contains the brick
 */
inline void
backpropagateNodes(Brick* brick,
                   Brick* lastBrick,
                   StaticSegment* segment)
{
    StaticNode* node = nullptr;
    float outH = 0.0f;
    StaticNode* lastNodes = &segment->nodes[lastBrick->nodePos];
    StaticNode* lastNode = nullptr;
    float* connections = nullptr;

    // iterate over all nodes within the brick
    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        node = &segment->nodes[nodeId];

        // step 1
        outH = node->value;
        node->delta *= outH * (1.0f - outH);

        connections = &segment->connections[node->targetConnectionPos];

        for(uint32_t j = 0; j < brick->numberOfNodes; j++)
        {
            lastNode = &lastNodes[j];

            // step 2
            lastNode->delta += node->delta * connections[j];

            // step 3
            connections[j] -= 0.1f * (node->delta * lastNode->value);
        }
    }
}

void
rewightStaticSegment(StaticSegment* segment)
{
    const uint32_t numberOfBricks = segment->segmentHeader->bricks.count;

    // run back-propagation over all internal nodes and synapses
    for(int32_t pos = numberOfBricks - 1; pos >= 0; pos--)
    {
        Brick* brick = &segment->bricks[pos];
        if(brick->isOutputBrick)
        {
            backpropagateOutput(brick, segment);
            backpropagateNodes(brick, &segment->bricks[pos - 1], segment);
        }
        else if(brick->isInputBrick)
        {
            backpropagateInput(brick, segment);
        }
        else
        {
            backpropagateNodes(brick, &segment->bricks[pos - 1], segment);
        }
    }
}

#endif // KYOUKOMIND_STATIC_BACKPROPAGATION_H
