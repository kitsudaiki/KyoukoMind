/**
 * @file        cluster_init.cpp
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

#include "cluster_init.h"


#include <core/data_structure/segments/dynamic_segment.h>
#include <core/data_structure/segments/static_segment.h>
#include <core/data_structure/segments/input_segment.h>
#include <core/data_structure/segments/output_segment.h>
#include <core/data_structure/cluster.h>

#include <core/objects/node.h>
#include <core/objects/synapses.h>

#include <core/routing_functions.h>
#include <core/data_structure/init/cluster_init.h>

#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiCommon/logger.h>

/**
 * @brief init header for a new cluster
 *
 * @param cluster pointer to the cluster where the header belongs to
 * @param metaData metadata-object to write into the header
 * @param settings settings-object to write into the header
 */
void
initHeader(Cluster* cluster,
           const ClusterMetaData &metaData,
           const ClusterSettings &settings)
{
    // allocate memory
    const uint32_t numberOfBlocks = 1;
    Kitsunemimi::allocateBlocks_DataBuffer(cluster->clusterData, numberOfBlocks);

    uint8_t* dataPtr = static_cast<uint8_t*>(cluster->clusterData.data);
    uint64_t pos = 0;

    // write metadata to buffer
    cluster->networkMetaData = reinterpret_cast<ClusterMetaData*>(dataPtr + pos);
    cluster->networkMetaData[0] = metaData;
    pos += sizeof(ClusterMetaData);

    // write settings to buffer
    cluster->networkSettings = reinterpret_cast<ClusterSettings*>(dataPtr + pos);
    cluster->networkSettings[0] = settings;
}

/**
 * @brief initalize new cluster
 *
 * @param cluster pointer to the uninitionalized cluster
 * @param parsedContent parsed json with the information of the cluster
 * @param uuid uuid for the new cluster
 *
 * @return true, if successful, else false
 */
bool
initNewCluster(Cluster* cluster,
               const JsonItem &parsedContent,
               const std::string &uuid)
{
    prepareSegments(parsedContent);

    const JsonItem paredSettings = parsedContent.get("settings");

    // meta-data
    ClusterMetaData newMetaData;
    strncpy(newMetaData.uuid.uuid, uuid.c_str(), 36);
    newMetaData.uuid.uuid[36] = '\0';

    // settings
    ClusterSettings newSettings;
    newSettings.cycleTime = paredSettings.get("cycle_time").getLong();

    initHeader(cluster, newMetaData, newSettings);

    const std::string name = parsedContent.get("name").getString();
    //const bool ret = cluster->setName(name);  // TODO: handle return

    LOG_INFO("create new cluster with uuid: " + cluster->networkMetaData->uuid.toString());

    // parse and create segments
    JsonItem segments = parsedContent.get("segments");
    for(uint32_t i = 0; i < segments.size(); i++)
    {
        const JsonItem segmentDef = segments.get(i);
        AbstractSegment* newSegment = nullptr;
        if(segmentDef.get("type").getString() == "dynamic_segment") {
            newSegment = addDynamicSegment(cluster, segmentDef);
        }
        if(segmentDef.get("type").getString() == "static_segment") {
            newSegment = addStaticSegment(cluster, segmentDef);
        }
        if(segmentDef.get("type").getString() == "input_segment") {
            newSegment = addInputSegment(cluster, segmentDef);
        }
        if(segmentDef.get("type").getString() == "output_segment") {
            newSegment = addOutputSegment(cluster, segmentDef);
        }

        if(newSegment == nullptr) {
            // TODO: error-handling
            return false;
        }

        // update segment information with cluster infos
        newSegment->segmentHeader->parentClusterId = cluster->networkMetaData->uuid;
        newSegment->parentCluster = cluster;
    }

    return true;
}

/**
 * @brief add new input-segment to cluster
 *
 * @param cluster pointer to the uninitionalized cluster
 * @param parsedContent parsed json with the information of the cluster
 *
 * @return true, if successful, else false
 */
AbstractSegment*
addInputSegment(Cluster* cluster,
                const JsonItem &parsedContent)
{
    InputSegment* newSegment = new InputSegment();

    if(newSegment->initSegment(parsedContent))
    {
        cluster->inputSegments.push_back(newSegment);
        cluster->allSegments.push_back(newSegment);
    }
    else
    {
        delete newSegment;
        newSegment = nullptr;
    }

    return newSegment;
}

/**
 * @brief add new output-segment to cluster
 *
 * @param cluster pointer to the uninitionalized cluster
 * @param parsedContent parsed json with the information of the cluster
 *
 * @return true, if successful, else false
 */
AbstractSegment*
addOutputSegment(Cluster* cluster,
                 const JsonItem &parsedContent)
{
    OutputSegment* newSegment = new OutputSegment();

    if(newSegment->initSegment(parsedContent))
    {
        cluster->outputSegments.push_back(newSegment);
        cluster->allSegments.push_back(newSegment);
    }
    else
    {
        delete newSegment;
        newSegment = nullptr;
    }

    return newSegment;
}

/**
 * @brief add new dynamic-segment to cluster
 *
 * @param cluster pointer to the uninitionalized cluster
 * @param parsedContent parsed json with the information of the cluster
 *
 * @return true, if successful, else false
 */
AbstractSegment*
addDynamicSegment(Cluster* cluster,
                  const JsonItem &parsedContent)
{
    DynamicSegment* newSegment = new DynamicSegment();

    if(newSegment->initSegment(parsedContent))
    {
        cluster->allSegments.push_back(newSegment);
    }
    else
    {
        delete newSegment;
        newSegment = nullptr;
    }

    return newSegment;
}

/**
 * @brief add new static-segment to cluster
 *
 * @param cluster pointer to the uninitionalized cluster
 * @param parsedContent parsed json with the information of the cluster
 *
 * @return true, if successful, else false
 */
AbstractSegment*
addStaticSegment(Cluster* cluster,
                 const JsonItem &parsedContent)
{
    StaticSegment* newSegment = new StaticSegment();

    if(newSegment->initSegment(parsedContent))
    {
        cluster->allSegments.push_back(newSegment);
    }
    else
    {
        delete newSegment;
        newSegment = nullptr;
    }

    return newSegment;
}

/**
 * @brief set directon for a segment for each side for later internal information-flow
 *
 * @param segments json with all segments
 * @param foundNext id of the neigbor-segment
 * @param side side where the neigbor is connected
 *
 * @return direction (input / output)
 */
const std::string
prepareDirection(const JsonItem &segments,
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
 * @brief get border-size between two connected segments
 *
 * @param currentSegment  json with current segment
 * @param segments json with all segments
 * @param foundNext id of the next segment
 *
 * @return size of the border
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
            // TODO: make configurable
            val = 500;
        }

        if(currentSegment.get("type").getString() == "static_segment"
                && segments.get(foundNext).get("type").getString() == "static_segment")
        {
            // TODO: make configurable
            val = 500;
        }

        if(currentSegment.get("type").getString() == "dynamic_segment"
                && segments.get(foundNext).get("type").getString() == "static_segment")
        {
            // TODO: make configurable
            val = 500;
        }

        if(currentSegment.get("type").getString() == "static_segment"
                && segments.get(foundNext).get("type").getString() == "dynamic_segment")
        {
            // TODO: make configurable
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
 * @brief prepare single segment
 *
 * @param segmentQueue reference to the segment-queue, which handles the segments for initializing
 * @param segments json with all segments
 * @param currentSegment current segment as json
 *
 * @return true, if successful, else false
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
        } else {
            direction = prepareDirection(segments, foundNext, side);
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
 * @brief prepare all segments
 *
 * @param parsedContent json with all segments
 *
 * @return true, if successful, else false
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

    // iterate over all input segments
    while(segmentQueue.size() > 0)
    {
        const uint32_t id = segmentQueue.front();
        segmentQueue.pop_front();
        JsonItem currentSegment = segments.get(id);

        prepareSingleSegment(segmentQueue, segments, currentSegment);
    }

    // debug-output
    // std::cout<<parsedContent.toString(true)<<std::endl;

    return true;
}

/**
 * @brief get id of the segment with match with the requested position
 *
 * @param segments json with all segments
 * @param nextPos requested position
 *
 * @return id of the segment, if found, else UNINIT_STATE_32
 */
uint32_t
checkNextPosition(const JsonItem &segments,
                  const Position nextPos)
{
    for(uint32_t i = 0; i < segments.size(); i++)
    {
        if(convertPosition(segments.get(i)) == nextPos) {
            return i;
        }
    }

    return UNINIT_STATE_32;
}

/**
 * @brief convert position information from json to position-item
 *
 * @param parsedContent json of the segment
 *
 * @return postion-objectfor the segment
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
