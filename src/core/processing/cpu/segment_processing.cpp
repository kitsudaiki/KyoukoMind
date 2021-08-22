/**
 * @file        synapse_processing.cpp
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

#include "segment_processing.h"

#include <core/processing/cpu/io.h>
#include <core/processing/cpu/backpropagation.h>
#include <core/processing/cpu/create_reduce.h>
#include <core/processing/cpu/processing.h>

/**
 * @brief process all input-nodes of a specific segment
 *
 * @param segment segment to process
 */
void
processSegmentInput(Segment* segment)
{
    processInputNodes(segment);
}

/**
 * @brief process all output-nodes of a specific segment
 *
 * @param segment segment to process
 */
void
processSegmentOutput(Segment* segment)
{
    processOutputNodes(segment);
}

 /**
 * @brief calculate the total error of all outputs of a specific segment
 *
 * @param segment segment to process
 *
 * @return total error value
 */
float
calculateSegmentError(Segment* segment)
{
    return calcTotalError(segment);
}

/**
 * @brief reduce all synapses within the segment and delete them, if the reach a deletion-border
 *
 * @param segment segment to process
 */
void
reduceSegment(Segment* segment)
{
    reduceSynapses(segment);
}

/**
 * @brief correct wight of synapses within
 *
 * @param segment segment to process
 */
void
rewightSegment(Segment* segment)
{
    const uint32_t numberOfBricks = segment->segmentHeader->bricks.count;

    // phase 1: correct new output-synapses
    for(int32_t pos = numberOfBricks - 1; pos >= 0; pos--)
    {
        const uint32_t brickId = segment->brickOrder[pos];
        Brick* brick = &segment->bricks[brickId];
        if(brick->isOutputBrick) {
            correctNewOutputSynapses(brick,segment);
        }
    }

    // phase 2: rewight synapses, which are directly connected to the output
    backpropagateOutput(segment);

    // run back-propagation over all internal nodes and synapses
    for(int32_t pos = numberOfBricks - 1; pos >= 0; pos--)
    {
        const uint32_t brickId = segment->brickOrder[pos];
        Brick* brick = &segment->bricks[brickId];
        backpropagateNodes(brick, segment);
    }
}

/**
 * @brief harden all synapses within a specific section
 *
 * @param segment segment to process
 */
void
hardenSegment(Segment* segment)
{
    hardenSynapses(segment);
}

/**
 * @brief process all nodes within a specific brick and also all synapse-sections,
 *        which are connected to an active node
 *
 * @param segment segment to process
 */
void
prcessSegment(Segment* segment)
{
    const uint32_t numberOfBricks = segment->segmentHeader->bricks.count;
    for(uint32_t pos = 0; pos < numberOfBricks; pos++)
    {
        const uint32_t brickId = segment->brickOrder[pos];
        Brick* brick = &segment->bricks[brickId];
        nodeProcessing(brick,
                       segment->nodes,
                       segment->synapseSections,
                       segment->bricks,
                       KyoukoRoot::m_networkCluster->randomValues,
                       segment->synapseSettings,
                       segment->dynamicBuffer);
    }
}
