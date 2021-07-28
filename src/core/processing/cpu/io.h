/**
 * @file        io.h
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

#ifndef CORE_IO_H
#define CORE_IO_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

/**
 * @brief process all input-nodes of a specific segment
 *
 * @param segment segment to process
 */
inline void
processInputNodes(Segment* segment)
{
    InputNode* inputNode = nullptr;

    for(uint64_t inputNodeId = 0;
        inputNodeId < segment->segmentHeader->inputs.count;
        inputNodeId++)
    {
        inputNode = &segment->inputs[inputNodeId];
        segment->nodes[inputNode->targetNode].input = inputNode->weight;
    }
}

/**
 * @brief process all output-nodes of a specific segment
 *
 * @param segment segment to process
 */
inline void
processOutputNodes(Segment* segment)
{
    OutputNode* out = nullptr;
    Node* targetNode = nullptr;
    float nodeWeight = 0.0f;

    for(uint64_t outputNodeId = 0;
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId++)
    {
        out = &segment->outputs[outputNodeId];
        targetNode = &segment->nodes[out->targetNode];
        nodeWeight = targetNode->potential;
        out->outputWeight = 1.0f / (1.0f + exp(-1.0f * nodeWeight));
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
calcTotalError(Segment* segment)
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

#endif // CORE_IO_H
