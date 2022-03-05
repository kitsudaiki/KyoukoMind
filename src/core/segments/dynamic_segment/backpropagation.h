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

#ifndef KYOUKOMIND_DYNAMIC_BACKPROPAGATION_H
#define KYOUKOMIND_DYNAMIC_BACKPROPAGATION_H

#include <common.h>

#include <kyouko_root.h>
#include <core/segments/brick.h>
#include <core/segments/dynamic_segment/dynamic_segment.h>

#include "objects.h"

/**
 * @brief calculate the total error of all outputs of a specific segment
 *
 * @param segment segment of which one the total error has to be calculated
 *
 * @return total error value
 */
inline float
calcTotalError(const Brick &brick,
               const DynamicSegment &segment)
{
    float totalError = 0.0f;
    DynamicNode* node = nullptr;
    float diff = 0.0f;

    for(uint32_t nodeId = brick.nodePos;
        nodeId < brick.numberOfNodes + brick.nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];
        diff = segment.inputTransfers[node->targetBorderId];
        totalError += 0.5f * (diff * diff);
    }

    return totalError;
}

/**
 * @brief backpropagate values of an output-brick
 *
 * @param brick brick to process
 * @param segment segment where the brick belongs to
 */
inline bool
backpropagateOutput(const Brick &brick,
                    const DynamicSegment &segment)
{
    DynamicNode* node = nullptr;
    float outH = 0.0f;

    if(calcTotalError(brick, segment) < 0.1f) {
        return false;
    }

    // iterate over all nodes within the brick
    for(uint32_t nodeId = brick.nodePos;
        nodeId < brick.numberOfNodes + brick.nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];
        node->delta = segment.inputTransfers[node->targetBorderId];
        outH = node->potential;
        node->delta *= outH * (1.0f - outH);
    }

    return true;
}

/**
 * @brief run back-propagation over the hidden neurons
 *
 * @param brick pointer to current brick
 * @param segment pointer to currect segment to process, which contains the brick
 */
inline void
backpropagateNodes(const Brick &brick,
                   const DynamicSegment &segment)
{
    uint16_t pos = 0;
    DynamicNode* sourceNode = nullptr;
    SynapseSection* section = nullptr;
    Synapse* synapse = nullptr;
    float netH = 0.0f;
    float outH = 0.0f;
    float learnValue = 0.2f;

    // iterate over all nodes within the brick
    for(uint32_t nodeId = brick.nodePos;
        nodeId < brick.numberOfNodes + brick.nodePos;
        nodeId++)
    {
        // skip section, if not active
        sourceNode = &segment.nodes[nodeId];
        if(sourceNode->targetSectionId == UNINIT_STATE_32) {
            continue;
        }
        section = &segment.synapseSections[sourceNode->targetSectionId];

        // set start-values
        pos = 0;
        netH = sourceNode->potential;
        outH = 1.0f / (1.0f + exp(-1.0f * netH));
        sourceNode->delta = 0.0f;

        // iterate over all synapses in the section
        while(pos < SYNAPSES_PER_SYNAPSESECTION
              && netH > 0.0f)
        {
            // break look, if no more synapses to process
            synapse = &section->synapses[pos];
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                break;
            }
            else if(synapse->targetNodeId == 0)
            {
                pos++;
                continue;
            }

            // update weight
            learnValue = static_cast<float>(100 - synapse->activeCounter) * 0.001f;
            learnValue += 0.1f;
            sourceNode->delta += segment.nodes[synapse->targetNodeId].delta * synapse->weight;
            synapse->weight -= learnValue * segment.nodes[synapse->targetNodeId].delta * outH;

            netH -= static_cast<float>(synapse->border) * BORDER_STEP;
            pos++;
        }

        if(brick.isInputBrick) {
            segment.outputTransfers[sourceNode->targetBorderId] = sourceNode->delta;
        }
    }
}

/**
 * @brief correct wight of synapses within
 *
 * @param segment segment to process
 */
void
rewightDynamicSegment(const DynamicSegment &segment)
{
    if(segment.dynamicSegmentSettings->doLearn == 0) {
        return;
    }

    // run back-propagation over all internal nodes and synapses
    const uint32_t numberOfBricks = segment.segmentHeader->bricks.count;
    for(int32_t pos = numberOfBricks - 1; pos >= 0; pos--)
    {
        const uint32_t brickId = segment.brickOrder[pos];
        Brick* brick = &segment.bricks[brickId];
        if(brick->isOutputBrick)
        {
            if(backpropagateOutput(*brick, segment) == false) {
                return;
            }
        }
        else
        {
            backpropagateNodes(*brick, segment);
        }
    }
}

#endif // KYOUKOMIND_DYNAMIC_BACKPROPAGATION_H
