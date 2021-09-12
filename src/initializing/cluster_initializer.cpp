/**
 * @file        cluster_initializer.cpp
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

#include "cluster_initializer.h"
#include <kyouko_root.h>

#include <core/objects/segments/dynamic_segment.h>
#include <core/objects/segments/input_segment.h>
#include <core/objects/segments/output_segment.h>

#include <core/objects/node.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

#include <core/processing/processing_unit_handler.h>
#include <core/processing/gpu/gpu_processing_uint.h>

#include <core/cluster_handler.h>
#include <initializing/routing_functions.h>

#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiPersistence/files/file_methods.h>
#include <libKitsunemimiPersistence/files/text_file.h>
#include <libKitsunemimiPersistence/logger/logger.h>

/**
 * @brief constructor
 */
ClusterInitializer::ClusterInitializer() {}

/**
 * @brief initialize new network
 *
 * @return true, if successfull, else false
 */
const std::string
ClusterInitializer::initCluster(const std::string &filePath)
{
    LOG_INFO("no files found. Try to create a new cluster");

    LOG_INFO("use init-file: " + filePath);

    std::string fileContent = "";
    std::string errorMessage = "";
    if(Kitsunemimi::Persistence::readFile(fileContent, filePath, errorMessage) == false)
    {
        LOG_ERROR(errorMessage);
        return std::string("");
    }

    // init randomizer
    srand(time(NULL));

    // check if values are valid
    if(fileContent == "") {
        return std::string("");
    }

    // parse input
    JsonItem parsedContent;
    const bool ret = parsedContent.parse(fileContent, errorMessage);
    if(ret == false)
    {
        LOG_ERROR("error while parsing input: " + errorMessage);
        return std::string("");
    }

    prepareSegments(parsedContent);
    std::cout<<parsedContent.toString(true)<<std::endl;
    const std::string uuid = createNewNetwork(parsedContent);
    if(uuid == "")
    {
        LOG_ERROR("failed to initialize network");
        return std::string("");
    }

    return uuid;
}

/**
 * @brief create blank new network
 *
 * @param fileContent file to parse with the basic structure of the network
 *
 * @return true, if successfull, else false
 */
const std::string
ClusterInitializer::createNewNetwork(const JsonItem &parsedContent)
{
    NetworkCluster newCluster;
    JsonItem paredSettings = parsedContent.get("settings");

    // network-meta
    newCluster.networkMetaData.cycleTime = paredSettings.get("cycle_time").getLong();

    NetworkCluster* cluster = new NetworkCluster();
    cluster->uuid = generateUuid();
    KyoukoRoot::m_root->m_clusterHandler->addCluster(cluster->uuid.toString(), cluster);

    LOG_INFO("create new cluster with uuid: " + cluster->uuid.toString());
    cluster->networkMetaData = newCluster.networkMetaData;

    JsonItem segments = parsedContent.get("segments");
    for(uint32_t i = 0; i < segments.size(); i++)
    {
        const JsonItem segmentDef = segments.get(i);
        AbstractSegment* newSegment = nullptr;
        if(segmentDef.get("type").getString() == "dynamic_segment") {
            newSegment = addDynamicSegment(segmentDef, cluster);
        }
        if(segmentDef.get("type").getString() == "input_segment") {
            newSegment = addInputSegment(segmentDef, cluster);
        }
        if(segmentDef.get("type").getString() == "output_segment") {
            newSegment = addOutputSegment(segmentDef, cluster);
        }

        if(newSegment != nullptr) {
            newSegment->segmentHeader->parentClusterId = cluster->uuid;
            newSegment->parentCluster = cluster;
        } else {
            // TODO: error-handling
        }
    }

    return cluster->uuid.toString();
}

/**
 * @brief ClusterInitializer::addInputSegment
 * @param parsedContent
 * @param cluster
 */
AbstractSegment*
ClusterInitializer::addInputSegment(const JsonItem &parsedContent,
                                    NetworkCluster* cluster)
{
    InputSegment* newSegment = new InputSegment();
    const bool ret = newSegment->initSegment(parsedContent);

    if(ret)
    {
        cluster->inputSegments.push_back(newSegment);
        cluster->allSegments.push_back(newSegment);
    }
    else
    {
        // TODO: handle error
    }

    return newSegment;
}

/**
 * @brief ClusterInitializer::addOutputSegment
 * @param parsedContent
 * @param cluster
 */
AbstractSegment*
ClusterInitializer::addOutputSegment(const JsonItem &parsedContent,
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

    return newSegment;
}

/**
 * @brief ClusterInitializer::addDynamicSegment
 * @param parsedContent
 * @param cluster
 */
AbstractSegment*
ClusterInitializer::addDynamicSegment(const JsonItem &parsedContent,
                                      NetworkCluster* cluster)
{
    DynamicSegment* newSegment = new DynamicSegment();
    const bool ret = newSegment->initSegment(parsedContent);
    if(ret) {
        cluster->allSegments.push_back(newSegment);
    } else {
        // TODO: handle error
    }

    return newSegment;
}

/**
 * @brief prepareSegments
 * @param parsedContent
 * @return
 */
bool
ClusterInitializer::prepareSegments(const JsonItem &parsedContent)
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
ClusterInitializer::checkSegments(const JsonItem &parsedContent,
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
