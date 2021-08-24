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

#ifndef CORE_BACKPROPAGATION_H
#define CORE_BACKPROPAGATION_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segments/output_segment.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

/**
 * @brief backpropagateOutput
 *
 * @param segment pointer to currect segment to process
 */
inline void
backpropagateOutput(OutputSegment* segment)
{
    float outW = 0.0f;
    OutputNode* out = nullptr;
    Node* targetNode = nullptr;

    // iterate over all output-nodes
    for(uint64_t outputNodeId = 0;
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId++)
    {
        out = &segment->outputs[outputNodeId];
        targetNode = &segment->nodes[out->targetNode];
        outW = out->outputWeight;
        targetNode->delta = (outW - out->shouldValue) * outW * (1.0f - outW);
    }
}

/**
 * @brief correct new created synapses, which are directly connected to the output and fix the
 *        sign of the values based on the should-value, to force the output in the right direction
 *
 * @param brick pointer to output-brick
 * @param segment pointer to currect segment to process, which contains the brick
 */
inline void
correctNewOutputSynapses(OutputSegment* segment)
{
    uint16_t pos = 0;
    Node* sourceNode = nullptr;
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
        if(section->active == Kitsunemimi::ItemBuffer::DELETED_SECTION) {
            continue;
        }

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
 * @brief correct wight of synapses within
 *
 * @param segment segment to process
 */
void
rewightSegment(OutputSegment* segment)
{
    correctNewOutputSynapses(segment);
    backpropagateOutput(segment);
}

#endif // CORE_BACKPROPAGATION_H
