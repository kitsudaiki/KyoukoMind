/**
 * @file        cluster_parsing.cpp
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

#include "preprocess_cluster_json.h"

#include <core/initializing/routing_functions.h>
#include <core/objects/cluster_meta.h>

#include <libKitsunemimiCommon/logger.h>


/**
 * @brief NetworkCluster::prepareDirection
 * @param currentSegment
 * @param segments
 * @param foundNext
 * @return
 */
const std::string
prepareDirection(const JsonItem &currentSegment,
                 const JsonItem &segments,
                 const uint32_t foundNext,
                 const uint8_t side)
{
    std::string direction = "";

    if(foundNext != UNINIT_STATE_32)
    {
        JsonItem nextSegment = segments.get(foundNext);
        if(nextSegment.contains("neighbors") == false)
        {
            direction = "output";
        }
        else
        {
            const JsonItem neighbor = nextSegment.get("neighbors").get(11 - side);
            const std::string otherDirection = neighbor.get("direction").getString();
            if(otherDirection == "input") {
                direction = "output";
            } else if(otherDirection == "output") {
                direction = "input";
            } else {
                assert(false);
            }
        }
    }

    return direction;
}

/**
 * @brief NetworkCluster::getNeighborBorderSize
 * @param currentSegment
 * @param segments
 * @param foundNext
 * @return
 */
long
getNeighborBorderSize(const JsonItem &currentSegment,
                      const JsonItem &segments,
                      const uint32_t foundNext)
{
    long val = 0;

    if(foundNext != UNINIT_STATE_32)
    {
        if(currentSegment.get("type").getString() == "dynamic_segment"
                && segments.get(foundNext).get("type").getString() == "dynamic_segment")
        {
            val = 500;
        }

        if(currentSegment.get("type").getString() == "input_segment") {
            val = currentSegment.get("number_of_inputs").getInt();
        }

        if(currentSegment.get("type").getString() == "output_segment") {
            val = currentSegment.get("number_of_outputs").getInt();
        }

        if(segments.get(foundNext).get("type").getString() == "output_segment") {
            val = segments.get(foundNext).get("number_of_outputs").getInt();
        }

        if(segments.get(foundNext).get("type").getString() == "input_segment") {
            val = segments.get(foundNext).get("number_of_inputs").getInt();
        }
    }

    return val;
}

/**
 * @brief NetworkCluster::prepareSingleSegment
 * @param parsedContent
 * @param parsedSegments
 * @return
 */
bool
prepareSingleSegment(std::deque<uint32_t> &segmentQueue,
                     const JsonItem &segments,
                     JsonItem &currentSegment)
{
    const Position currentPosition = convertPosition(currentSegment);
    DataArray* nextList = new DataArray();
    long borderBufferSize = 0;

    for(uint32_t side = 0; side < 12; side++)
    {
        const Position nextPos = getNeighborPos(currentPosition, side);
        const uint32_t foundNext = checkNextPosition(segments, nextPos);
        DataMap* neighborSettings = new DataMap();

        // add next segment to initializing-queue, if not already processed
        if(foundNext != UNINIT_STATE_32
                && segments.get(foundNext).contains("neighbors") == false)
        {
            segmentQueue.push_back(foundNext);
        }

        // get id of the next
        neighborSettings->insert("id", new DataValue(static_cast<long>(foundNext)));

        // get size of the border for the side
        const long val = getNeighborBorderSize(currentSegment, segments, foundNext);
        borderBufferSize += val;
        neighborSettings->insert("size", new DataValue(val));

        // get direction of the side
        std::string direction = "";
        if(foundNext != UNINIT_STATE_32
                && currentSegment.get("type").getString() == "input_segment")
        {
            direction = "output";
        }
        else
        {
            direction = prepareDirection(currentSegment,
                                         segments,
                                         foundNext,
                                         side);
        }
        neighborSettings->insert("direction", new DataValue(direction));

        nextList->append(neighborSettings);
    }

    // update segment with the new collected data
    currentSegment.insert("neighbors", nextList);
    currentSegment.insert("total_border_size", new DataValue(borderBufferSize));

    return true;
}

/**
 * @brief prepareSegments
 * @param parsedContent
 * @return
 */
bool
prepareSegments(const JsonItem &parsedContent)
{
    std::deque<uint32_t> segmentQueue;
    JsonItem segments = parsedContent.get("segments");

    // search input-segments
    for(uint32_t i = 0; i < segments.size(); i++)
    {
        JsonItem currentSegment = segments.get(i);
        if(currentSegment.get("type").getString() == "input_segment") {
            segmentQueue.push_back(i);
        }
    }

    // iterate over all segments
    while(segmentQueue.size() > 0)
    {
        const uint32_t id = segmentQueue.front();
        segmentQueue.pop_front();
        JsonItem currentSegment = segments.get(id);

        prepareSingleSegment(segmentQueue, segments, currentSegment);
    }

    std::cout<<parsedContent.toString(true)<<std::endl;

    return true;
}

/**
 * @brief ClusterInitializer::checkSegments
 * @param parsedContent
 * @param nextPos
 * @return
 */
uint32_t
checkNextPosition(const JsonItem &segments,
                  const Position nextPos)
{
    for(uint32_t i = 0; i < segments.size(); i++)
    {
        const JsonItem parsedPosition = segments.get(i).get("position");
        Position currentPosition;
        currentPosition.x = parsedPosition.get(0).getInt();
        currentPosition.y = parsedPosition.get(1).getInt();
        currentPosition.z = parsedPosition.get(2).getInt();

        if(currentPosition == nextPos) {
            return i;
        }
    }

    return UNINIT_STATE_32;
}

/**
 * @brief NetworkCluster::convertPosition
 * @param parsedContent
 * @return
 */
Position
convertPosition(const JsonItem &parsedContent)
{
    JsonItem parsedPosition = parsedContent.get("position");
    Position currentPosition;
    currentPosition.x = parsedPosition.get(0).getInt();
    currentPosition.y = parsedPosition.get(1).getInt();
    currentPosition.z = parsedPosition.get(2).getInt();

    return currentPosition;
}
