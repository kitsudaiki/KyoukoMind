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
 * @brief segmentInputProcessing
 * @param segment
 */
void
processSegmentInput(Segment* segment)
{
    processInputNodes(segment);
}

/**
 * @brief segmentOutputProcessing
 * @param segment
 */
void
processSegmentOutput(Segment* segment)
{
    processOutputNodes(segment);
}

 /**
 * @brief segmentCalculateError
 * @param segment
 * @return
 */
float
calculateSegmentError(Segment* segment)
{
    return calcTotalError(segment);
}

/**
 * @brief segmentReduceSynapses
 * @param segment
 */
void
reduceSegmentSynapses(Segment* segment)
{
    reduceCoreSynapses(segment);
}

/**
 * @brief segmentBackpropagation
 * @param segment
 */
void
rewightSegment(Segment* segment)
{
    const uint32_t numberOfBricks = segment->segmentHeader->bricks.count;

    for(int32_t pos = numberOfBricks - 1; pos >= 0; pos--)
    {
        const uint32_t brickId = segment->brickOrder[pos];
        Brick* brick = &segment->bricks[brickId];
        if(brick->isOutputBrick) {
            correctNewOutputSynapses(brick,segment);
        }
    }

    backpropagateOutput(segment);

    for(int32_t pos = numberOfBricks - 1; pos >= 0; pos--)
    {
        const uint32_t brickId = segment->brickOrder[pos];
        Brick* brick = &segment->bricks[brickId];
        backpropagateNodes(brick, segment);
    }
}

/**
 * @brief segmentHardeing
 * @param segment
 */
void
hardenSegment(Segment* segment)
{
    hardenSynapses(segment);
}

/**
 * @brief segmentNodeProcessing
 * @param segment
 */
void
prcessSegmentNodes(Segment* segment)
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
                       &KyoukoRoot::m_networkCluster->networkMetaData);
    }
}
