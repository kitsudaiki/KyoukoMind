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
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/data_structure/segments/dynamic_segment.h>
#include <core/objects/synapses.h>

/**
 * @brief correct new created synapses, which are directly connected to the output and fix the
 *        sign of the values based on the should-value, to force the output in the right direction
 *
 * @param brick pointer to output-brick
 * @param segment pointer to currect segment to process, which contains the brick
 */
inline void
correctNewOutputSynapses(Brick* brick,
                         DynamicSegment* segment)
{
    uint16_t pos = 0;
    DynamicNode* sourceNode = nullptr;
    SynapseSection* section = nullptr;
    Synapse* synapse = nullptr;
    float netH = 0.0f;
    float delta = 0.0f;
    float invert = 0.0f;

    // iterate over all nodes within the brick
    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        // skip section, if not active
        sourceNode = &segment->nodes[nodeId];
        if(sourceNode->targetSectionId == UNINIT_STATE_32) {
            continue;
        }
        section = &segment->synapseSections[sourceNode->targetSectionId];

        // set start-values
        pos = section->hardening;
        netH = sourceNode->potential;

        // iterate over all synapses in the section
        while(pos < SYNAPSES_PER_SYNAPSESECTION
              && netH > 0.0f)
        {
            // break look, if no more synapses to process
            synapse = &section->synapses[pos];
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                break;
            }

            // correct weight-value, if necessary
            delta = segment->nodes[synapse->targetNodeId].delta;
            invert = (delta < 0.0f && synapse->weight < 0.0f)
                     || (delta > 0.0f && synapse->weight > 0.0f);
            synapse->weight += -2.0f * invert * synapse->weight;

            // update loop-counter
            netH -= static_cast<float>(synapse->border) * BORDER_STEP;
            pos++;
        }
    }
}

/**
 * @brief backpropagate values of an output-brick
 *
 * @param brick brick to process
 * @param segment segment where the brick belongs to
 */
inline void
backpropagateOutput(Brick* brick,
                    DynamicSegment* segment)
{
    DynamicNode* node = nullptr;
    float outH = 0.0f;

    // iterate over all nodes within the brick
    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        node = &segment->nodes[nodeId];
        node->delta = segment->inputTransfers[node->targetBorderId];
        outH = node->potential;
        node->delta *= outH * (1.0f - outH);
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
                   DynamicSegment* segment)
{
    uint16_t pos = 0;
    DynamicNode* sourceNode = nullptr;
    SynapseSection* section = nullptr;
    Synapse* synapse = nullptr;
    float netH = 0.0f;
    float outH = 0.0f;
    float learnValue = 0.0f;

    // iterate over all nodes within the brick
    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        // skip section, if not active
        sourceNode = &segment->nodes[nodeId];
        if(sourceNode->targetSectionId == UNINIT_STATE_32) {
            continue;
        }
        section = &segment->synapseSections[sourceNode->targetSectionId];

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

            // update weight
            learnValue = static_cast<float>(pos <= section->hardening) * 0.1f
                         + static_cast<float>(pos > section->hardening) * 0.2f;
            sourceNode->delta += segment->nodes[synapse->targetNodeId].delta * synapse->weight;
            synapse->weight -= learnValue * segment->nodes[synapse->targetNodeId].delta * outH;

            // update loop-counter
            netH -= static_cast<float>(synapse->border) * BORDER_STEP;
            pos++;
        }

        if(brick->isInputBrick) {
            segment->outputTransfers[sourceNode->targetBorderId] = sourceNode->delta;
        }
    }
}

/**
 * @brief correct wight of synapses within
 *
 * @param segment segment to process
 */
void
rewightDynamicSegment(DynamicSegment* segment)
{
    const uint32_t numberOfBricks = segment->segmentHeader->bricks.count;

    // run back-propagation over all internal nodes and synapses
    for(int32_t pos = numberOfBricks - 1; pos >= 0; pos--)
    {
        const uint32_t brickId = segment->brickOrder[pos];
        Brick* brick = &segment->bricks[brickId];
        if(brick->isOutputBrick) {
            backpropagateOutput(brick, segment);
        } else {
            backpropagateNodes(brick, segment);
        }
    }
}

#endif // KYOUKOMIND_DYNAMIC_BACKPROPAGATION_H
