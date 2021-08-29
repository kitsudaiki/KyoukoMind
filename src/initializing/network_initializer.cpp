/**
 * @file        network_initializer.cpp
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

#include "network_initializer.h"
#include <kyouko_root.h>

#include <core/objects/segments/dynamic_segment.h>
#include <core/objects/segments/input_segment.h>
#include <core/objects/segments/output_segment.h>

#include <core/objects/node.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

#include <core/processing/processing_unit_handler.h>
#include <core/processing/gpu/gpu_processing_uint.h>

#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiPersistence/files/file_methods.h>
#include <libKitsunemimiPersistence/files/text_file.h>
#include <libKitsunemimiPersistence/logger/logger.h>

/**
 * @brief constructor
 */
ClusterInitializer::ClusterInitializer()
{
}

/**
 * @brief initialize new network
 *
 * @return true, if successfull, else false
 */
bool
ClusterInitializer::initNetwork(const std::string &filePath)
{
    bool success = false;

    LOG_INFO("no files found. Try to create a new cluster");

    LOG_INFO("use init-file: " + filePath);

    std::string fileContent = "";
    std::string errorMessage = "";
    if(Kitsunemimi::Persistence::readFile(fileContent, filePath, errorMessage) == false)
    {
        LOG_ERROR(errorMessage);
        return false;
    }

    // init randomizer
    srand(time(NULL));

    // check if values are valid
    if(fileContent == "") {
        return false;
    }

    // parse input
    JsonItem parsedContent;
    const bool ret = parsedContent.parse(fileContent, errorMessage);
    if(ret == false)
    {
        LOG_ERROR("error while parsing input: " + errorMessage);
        return false;
    }

    prepareSegments(parsedContent);
    //std::cout<<parsedContent.toString(true)<<std::endl;
    success = createNewNetwork(parsedContent);
    if(success == false)
    {
        LOG_ERROR("failed to initialize network");
        return false;
    }

    return true;
}

/**
 * @brief create blank new network
 *
 * @param fileContent file to parse with the basic structure of the network
 *
 * @return true, if successfull, else false
 */
bool
ClusterInitializer::createNewNetwork(JsonItem &parsedContent)
{

    NetworkCluster newCluster;
    JsonItem paredSettings = parsedContent["settings"];

    // network-meta
    newCluster.networkMetaData.cycleTime = paredSettings.get("cycle_time").getLong();
    newCluster.initMetaData.maxBrickDistance = paredSettings["max_brick_distance"].getInt();
    newCluster.initMetaData.maxSynapseSections = paredSettings["max_synapse_sections"].getLong();

    KyoukoRoot::m_networkCluster = new NetworkCluster();
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;
    cluster->initMetaData = newCluster.initMetaData;
    cluster->networkMetaData = newCluster.networkMetaData;

    JsonItem segments = parsedContent["segments"];
    for(uint32_t i = 0; i < segments.size(); i++)
    {
        JsonItem currentSegment = segments[i];
        if(currentSegment["type"].getString() == "dynamic_segment") {
            addDynamicSegment(currentSegment, cluster);
        }
        if(currentSegment["type"].getString() == "input_segment") {
            addInputSegment(currentSegment, cluster);
        }
        if(currentSegment["type"].getString() == "output_segment") {
            addOutputSegment(currentSegment, cluster);
        }
    }

    return true;
}

/**
 * @brief ClusterInitializer::addInputSegment
 * @param parsedContent
 * @param cluster
 */
void
ClusterInitializer::addInputSegment(JsonItem &parsedContent,
                                    NetworkCluster* cluster)
{
    InputSegment* newSegment = new InputSegment();
    const bool ret = newSegment->initSegment(parsedContent);

    if(ret) {
        cluster->inputSegments.push_back(newSegment);
        cluster->allSegments.push_back(newSegment);
    } else {
        // TODO: handle error
    }
}

/**
 * @brief ClusterInitializer::addOutputSegment
 * @param parsedContent
 * @param cluster
 */
void
ClusterInitializer::addOutputSegment(JsonItem &parsedContent,
                                     NetworkCluster* cluster)
{
    OutputSegment* newSegment = new OutputSegment();
    const bool ret = newSegment->initSegment(parsedContent);

    if(ret) {
        cluster->outputSegments.push_back(newSegment);
        cluster->allSegments.push_back(newSegment);
    } else {
        // TODO: handle error
    }
}

/**
 * @brief ClusterInitializer::addDynamicSegment
 * @param parsedContent
 * @param cluster
 */
void
ClusterInitializer::addDynamicSegment(JsonItem &parsedContent,
                                      NetworkCluster* cluster)
{
    DynamicSegment* newSegment = new DynamicSegment();
    const bool ret = newSegment->initSegment(parsedContent);
    if(ret) {
        cluster->allSegments.push_back(newSegment);
    } else {
        // TODO: handle error
    }
}

/**
 * @brief get the next position in the raster for a specific brick and side
 *
 * @param x current x-position
 * @param y current y-position
 * @param z current z-position
 * @param side side to go to next
 *
 * @return position of the next brick for the specific side
 */
Position
ClusterInitializer::getNeighborPos(const Position sourcePos, const uint8_t side)
{
    Position result;
    result.x = UNINIT_STATE_32;
    result.y = UNINIT_STATE_32;
    result.z = UNINIT_STATE_32;

    switch (side)
    {
    case 0:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z - 1;
            break;
        }
    case 1:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z - 1;
            break;
        }
    case 2:
        {
            result.x = sourcePos.x;
            result.y = sourcePos.y;
            result.z = sourcePos.z - 1;
            break;
        }
    case 3:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z;
            break;
        }
    case 4:
        {
            result.x = sourcePos.x + 1;
            result.y = sourcePos.y;
            result.z = sourcePos.z;
            break;
        }
    case 5:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z;
            break;
        }
    case 8:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z;
            break;
        }
    case 7:
        {
            result.x = sourcePos.x - 1;
            result.y = sourcePos.y;
            result.z = sourcePos.z;
            break;
        }
    case 6:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z;
            break;
        }
    case 9:
        {
            result.x = sourcePos.x;
            result.y = sourcePos.y;
            result.z = sourcePos.z + 1;
            break;
        }
    case 10:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z + 1;
            break;
        }
    case 11:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z + 1;
            break;
        }
    default:
        assert(false);
    }
    return result;
}

/**
 * @brief prepareSegments
 * @param parsedContent
 * @return
 */
bool
ClusterInitializer::prepareSegments(JsonItem &parsedContent)
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
        DataArray* sizeList = new DataArray();

        for(uint32_t side = 0; side < 12; side++)
        {
            const Position nextPos = getNeighborPos(currentPosition, side);
            const long foundNext = checkSegments(parsedContent, nextPos);
            nextList->append(new DataValue(foundNext));

            if(foundNext != UNINIT_STATE_32)
            {
                if(currentSegment.get("type").getString() == "dynamic_segment"
                        && segments.get(foundNext).get("type").getString() == "dynamic_segment")
                {
                    sizeList->append(new DataValue(500)); // TODO: make value configurable
                    continue;
                }

                if(currentSegment.get("type").getString() == "output_segment")
                {
                    const long val = currentSegment.get("number_of_outputs").getInt();
                    sizeList->append(new DataValue(val));
                    continue;
                }

                if(segments.get(foundNext).get("type").getString() == "output_segment")
                {
                    const long val = segments.get(foundNext).get("number_of_outputs").getInt();
                    sizeList->append(new DataValue(val));
                    continue;
                }

                if(currentSegment.get("type").getString() == "input_segment")
                {
                    const long val = currentSegment.get("number_of_inputs").getInt();
                    sizeList->append(new DataValue(val));
                    continue;
                }

                if(segments.get(foundNext).get("type").getString() == "input_segment")
                {
                    const long val = segments.get(foundNext).get("number_of_inputs").getInt();
                    sizeList->append(new DataValue(val));
                    continue;
                }
            }
            else
            {
                sizeList->append(new DataValue(0));
            }
        }

        currentSegment.insert("sides", nextList);
        currentSegment.insert("border_size", sizeList);
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
ClusterInitializer::checkSegments(JsonItem &parsedContent,
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
