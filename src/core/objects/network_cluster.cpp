/**
 * @file        network_cluster.cpp
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

#include "network_cluster.h"

#include <core/objects/segments/dynamic_segment.h>
#include <core/objects/segments/input_segment.h>
#include <core/objects/segments/output_segment.h>

#include <core/objects/node.h>
#include <core/objects/synapses.h>

#include <core/routing_functions.h>

#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiPersistence/logger/logger.h>

NetworkCluster::NetworkCluster() {}

/**
 * @brief InputSegment::initSegmentPointer
 * @param header
 */
void
NetworkCluster::initSegmentPointer(const ClusterMetaData &metaData,
                                   const ClusterSettings &settings)
{
    const uint32_t numberOfBlocks = 1;
    Kitsunemimi::allocateBlocks_DataBuffer(clusterData, numberOfBlocks);

    uint8_t* dataPtr = static_cast<uint8_t*>(clusterData.data);
    uint64_t pos = 0;

    networkMetaData = reinterpret_cast<ClusterMetaData*>(dataPtr + pos);
    networkMetaData[0] = metaData;

    pos += sizeof(ClusterMetaData);
    networkSettings = reinterpret_cast<ClusterSettings*>(dataPtr + pos);
    networkSettings[0] = settings;
}

/**
 * @brief create blank new network
 *
 * @param fileContent file to parse with the basic structure of the network
 *
 * @return true, if successfull, else false
 */
const std::string
NetworkCluster::initNewCluster(const JsonItem &parsedContent)
{
    prepareSegments(parsedContent);

    JsonItem paredSettings = parsedContent.get("settings");

    // network-meta
    ClusterMetaData newMetaData;
    newMetaData.uuid = generateUuid();

    ClusterSettings newSettings;
    newSettings.cycleTime = paredSettings.get("cycle_time").getLong();

    initSegmentPointer(newMetaData, newSettings);

    const std::string name = parsedContent.get("name").getString();
    const bool ret = setName(name);  // TODO: handle return

    LOG_INFO("create new cluster with uuid: " + networkMetaData->uuid.toString());

    JsonItem segments = parsedContent.get("segments");
    for(uint32_t i = 0; i < segments.size(); i++)
    {
        const JsonItem segmentDef = segments.get(i);
        AbstractSegment* newSegment = nullptr;
        if(segmentDef.get("type").getString() == "dynamic_segment") {
            newSegment = addDynamicSegment(segmentDef);
        }
        if(segmentDef.get("type").getString() == "input_segment") {
            newSegment = addInputSegment(segmentDef);
        }
        if(segmentDef.get("type").getString() == "output_segment") {
            newSegment = addOutputSegment(segmentDef);
        }

        if(newSegment == nullptr) {
            // TODO: error-handling
        }

        // update segment information with cluster infos
        newSegment->segmentHeader->parentClusterId = networkMetaData->uuid;
        newSegment->parentCluster = this;
    }

    return networkMetaData->uuid.toString();
}

/**
 * @brief ClusterInitializer::addInputSegment
 * @param parsedContent
 * @param cluster
 */
AbstractSegment*
NetworkCluster::addInputSegment(const JsonItem &parsedContent)
{
    InputSegment* newSegment = new InputSegment();
    const bool ret = newSegment->initSegment(parsedContent);

    if(ret)
    {
        inputSegments.push_back(newSegment);
        allSegments.push_back(newSegment);
    }
    else
    {
        delete newSegment;
        newSegment = nullptr;
    }

    return newSegment;
}

/**
 * @brief ClusterInitializer::addOutputSegment
 * @param parsedContent
 * @param cluster
 */
AbstractSegment*
NetworkCluster::addOutputSegment(const JsonItem &parsedContent)
{
    OutputSegment* newSegment = new OutputSegment();
    const bool ret = newSegment->initSegment(parsedContent);

    if(ret)
    {
        outputSegments.push_back(newSegment);
        allSegments.push_back(newSegment);
    }
    else
    {
        delete newSegment;
        newSegment = nullptr;
    }

    return newSegment;
}

/**
 * @brief ClusterInitializer::addDynamicSegment
 * @param parsedContent
 * @param cluster
 */
AbstractSegment*
NetworkCluster::addDynamicSegment(const JsonItem &parsedContent)
{
    DynamicSegment* newSegment = new DynamicSegment();
    const bool ret = newSegment->initSegment(parsedContent);
    if(ret)
    {
        allSegments.push_back(newSegment);
    }
    else
    {
        delete newSegment;
        newSegment = nullptr;
    }

    return newSegment;
}

/**
 * @brief NetworkCluster::getName
 * @return
 */
const std::string
NetworkCluster::getName()
{
    // precheck
    if(networkMetaData == nullptr) {
        return std::string("");
    }

    const std::string nameStr = std::string(networkMetaData->name);
    assert(nameStr.size() <= 1024);

    return nameStr;
}

/**
 * @brief NetworkCluster::setName
 * @param newName
 * @return
 */
bool
NetworkCluster::setName(const std::string newName)
{
    // precheck
    if(networkMetaData == nullptr
            || newName.size() > 1023
            || newName.size() == 0)
    {
        return false;
    }

    // copy string into char-buffer and set explicit the escape symbol to be absolut sure
    // that it is set to absolut avoid buffer-overflows
    strncpy(networkMetaData->name, newName.c_str(), newName.size());
    networkMetaData->name[newName.size()] = '\0';

    return true;
}

/**
 * @brief prepareSegments
 * @param parsedContent
 * @return
 */
bool
NetworkCluster::prepareSegments(const JsonItem &parsedContent)
{
    JsonItem segments = parsedContent.get("segments");
    for(uint32_t i = 0; i < segments.size(); i++)
    {
        JsonItem currentSegment = segments.get(i);

        JsonItem parsedPosition = currentSegment.get("position");
        Position currentPosition;
        currentPosition.x = parsedPosition.get(0).getInt();
        currentPosition.y = parsedPosition.get(1).getInt();
        currentPosition.z = parsedPosition.get(2).getInt();

        DataArray* nextList = new DataArray();
        long borderBufferSize = 0;

        for(uint32_t side = 0; side < 12; side++)
        {
            const Position nextPos = getNeighborPos(currentPosition, side);
            const long foundNext = checkSegments(parsedContent, nextPos);

            DataMap* neighborSettings = new DataMap();
            neighborSettings->insert("id", new DataValue(foundNext));

            long val = 0;
            std::string direction = "";

            if(foundNext != UNINIT_STATE_32)
            {
                if(currentSegment.get("type").getString() == "dynamic_segment"
                        && segments.get(foundNext).get("type").getString() == "dynamic_segment")
                {
                    val = 500;
                }

                if(currentSegment.get("type").getString() == "output_segment")
                {
                    val = currentSegment.get("number_of_outputs").getInt();
                    direction = "input";
                }

                if(segments.get(foundNext).get("type").getString() == "output_segment")
                {
                    val = segments.get(foundNext).get("number_of_outputs").getInt();
                    direction = "output";
                }

                if(currentSegment.get("type").getString() == "input_segment")
                {
                    val = currentSegment.get("number_of_inputs").getInt();
                    direction = "output";
                }

                if(segments.get(foundNext).get("type").getString() == "input_segment")
                {
                    val = segments.get(foundNext).get("number_of_inputs").getInt();
                    direction = "input";
                }
            }

            neighborSettings->insert("size", new DataValue(val));
            neighborSettings->insert("direction", new DataValue(direction));
            borderBufferSize += val;

            nextList->append(neighborSettings);
        }

        currentSegment.insert("neighbors", nextList);
        currentSegment.insert("total_border_size", new DataValue(borderBufferSize));
    }

    return true;
}

/**
 * @brief ClusterInitializer::checkSegments
 * @param parsedContent
 * @param nextPos
 * @return
 */
uint32_t
NetworkCluster::checkSegments(const JsonItem &parsedContent,
                              const Position nextPos)
{
    JsonItem segments = parsedContent.get("segments");
    for(uint32_t i = 0; i < segments.size(); i++)
    {
        JsonItem parsedPosition = segments.get(i).get("position");
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
