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
#include <core/segments/brick.h>
#include <core/cluster/cluster.h>

#include "objects.h"
#include "output_segment.h"

/**
 * @brief get position of the highest output-position
 *
 * @param segment output-segment to check
 *
 * @return position of the highest output.
 */
inline uint32_t
getHighestOutput(const OutputSegment &segment)
{
    float hightest = -0.1f;
    uint32_t hightestPos = 0;
    OutputNode* out = nullptr;

    for(uint32_t outputNodeId = 0;
        outputNodeId < segment.segmentHeader->outputs.count;
        outputNodeId++)
    {
        out = &segment.outputs[outputNodeId];
        if(out->outputWeight > hightest)
        {
            hightest = out->outputWeight;
            hightestPos = outputNodeId;
        }
    }

    //std::cout<<"hightes: "<<hightestPos<<std::endl;
    return hightestPos;
}

/**
 * @brief process all nodes within a specific brick and also all synapse-sections,
 *        which are connected to an active node
 *
 * @param segment segment to process
 */
inline void
prcessOutputSegment(const OutputSegment &segment)
{
    OutputNode* node = nullptr;
    const float doLearn = static_cast<float>(segment.dynamicSegmentSettings->doLearn);

    float* inputTransfers = segment.inputTransfers;
    for(uint64_t outputNodeId = 0;
        outputNodeId < segment.segmentHeader->outputs.count;
        outputNodeId++)
    {
        node = &segment.outputs[outputNodeId];
        node->outputWeight = inputTransfers[node->targetBorderId];
        segment.outputTransfers[node->targetBorderId] = (node->outputWeight - node->shouldValue)
                                                        * doLearn;
    }
}

#endif // KYOUKOMIND_OUTPUT_PROCESSING_H
