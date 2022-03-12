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
    float totalDelta = 0.0f;

    // iterate over all nodes within the brick
    for(uint32_t nodeId = brick.nodePos;
        nodeId < brick.numberOfNodes + brick.nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];
        node->delta = segment.inputTransfers[node->targetBorderId];
        totalDelta += node->delta;
    }

    return totalDelta > 0.01f || totalDelta < -0.01f;
}

/**
 * @brief run backpropagation for a single synapse-section
 *
 * @param section pointer to section to process
 * @param sourceNode pointer to the node, who triggered the section
 * @param netH node-potential
 * @param outH output-multiplicator
 * @param brick brick where the seciton is located
 * @param segment segment where section belongs to
 */
inline void
backpropagateSection(SynapseSection* section,
                     DynamicNode* sourceNode,
                     float netH,
                     const float outH,
                     const Brick &brick,
                     const DynamicSegment &segment)
{
    Synapse* synapse = nullptr;
    float learnValue = 0.2f;
    uint16_t pos = 0;

    // iterate over all synapses in the section
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && netH > 0.0f)
    {
        // break look, if no more synapses to process
        synapse = &section->synapses[pos];
        if(synapse->targetNodeId == UNINIT_STATE_16) {
            break;
        }
        // HINT: this condition is not necessary as long as the active-couter below is incomplete
        // else if(synapseObj.targetNodeId == 0)
        // {
        //     pos++;
        //     netH -= static_cast<float>(synapseObj.border) * BORDER_STEP;
        //     continue;
        // }

        // update weight
        //learnValue = static_cast<float>(100 - synapse->activeCounter) * 0.001f;
        learnValue = 0.1f;
        sourceNode->delta += segment.nodes[synapse->targetNodeId].delta * synapse->weight;
        synapse->weight -= learnValue * segment.nodes[synapse->targetNodeId].delta * outH;

        netH -= static_cast<float>(synapse->border) * BORDER_STEP;
        pos++;
    }

    if(section->next != UNINIT_STATE_32
            && netH > 0.1f)
    {
        backpropagateSection(&segment.synapseSections[section->next],
                             sourceNode,
                             netH,
                             outH,
                             brick,
                             segment);
    }
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
    DynamicNode* sourceNode = nullptr;
    SynapseSection* section = nullptr;

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
        const float netH = sourceNode->potential;
        const float outH = 1.0f / (1.0f + exp(-1.0f * netH));
        sourceNode->delta = 0.0f;

        backpropagateSection(section,
                             sourceNode,
                             netH,
                             outH,
                             brick,
                             segment);

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
