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

#ifndef KYOUKOMIND_OUTPUT_PROCESSING_H
#define KYOUKOMIND_OUTPUT_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segments/output_segment.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

/**
 * @brief process all nodes within a specific brick and also all synapse-sections,
 *        which are connected to an active node
 *
 * @param segment segment to process
 */
void
prcessOutputSegment(OutputSegment* segment)
{
    const uint32_t numberOfOutputs = segment->segmentHeader->outputs.count;
    float* inputTransfers = segment->inputTransfers;
    for(uint32_t pos = 0; pos < numberOfOutputs; pos++)
    {
        OutputNode* node = &segment->outputs[pos];
        node->outputWeight = inputTransfers[node->targetBorderId];
    }
}

/**
 * @brief calculate the total error of all outputs of a specific segment
 *
 * @param segment segment of which one the total error has to be calculated
 *
 * @return total error value
 */
inline float
calcTotalError(OutputSegment* segment)
{
    float totalError = 0.0f;
    OutputNode* out = nullptr;
    float diff = 0.0f;

    for(uint64_t outputNodeId = 0;
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId++)
    {
        out = &segment->outputs[outputNodeId];
        diff = (out->shouldValue - out->outputWeight);
        totalError += 0.5f * (diff * diff);
    }

    return totalError;
}

/**
 * @brief getHighestOutput
 * @param segment
 * @return
 */
uint64_t
getHighestOutput(OutputSegment* segment)
{
    float hightest = -0.1f;
    uint64_t hightestPos = 0;
    OutputNode* out = nullptr;

    for(uint64_t outputNodeId = 0;
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId++)
    {
        out = &segment->outputs[outputNodeId];
        if(out->outputWeight > hightest)
        {
            hightest = out->outputWeight;
            hightestPos = outputNodeId;
        }
    }

    return hightestPos;
}

#endif // KYOUKOMIND_OUTPUT_PROCESSING_H
