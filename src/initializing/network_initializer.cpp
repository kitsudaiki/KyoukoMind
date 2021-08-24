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

    success = createNewNetwork(fileContent);
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
ClusterInitializer::createNewNetwork(const std::string &fileContent)
{
    // init randomizer
    srand(time(NULL));

    // check if values are valid
    if(fileContent == "") {
        return false;
    }

    // parse input
    std::string errorMessage = "";
    JsonItem parsedContent;
    const bool ret = parsedContent.parse(fileContent, errorMessage);
    if(ret == false)
    {
        LOG_ERROR("error while parsing input: " + errorMessage);
        return false;
    }

    NetworkCluster newCluster;
    SegmentSettings settings;
    JsonItem paredSettings = parsedContent["settings"];

    // network-meta
    newCluster.networkMetaData.cycleTime = paredSettings.get("cycle_time").getLong();

    // init-meta
    newCluster.initMetaData.nodesPerBrick = paredSettings["nodes_per_brick"].getInt();
    newCluster.initMetaData.nodeLowerBorder = paredSettings["node_lower_border"].getFloat();
    newCluster.initMetaData.nodeUpperBorder = paredSettings["node_upper_border"].getFloat();
    newCluster.initMetaData.maxBrickDistance = paredSettings["max_brick_distance"].getInt();
    newCluster.initMetaData.maxSynapseSections = paredSettings["max_synapse_sections"].getLong();
    newCluster.initMetaData.layer = paredSettings["layer"].getInt();

    // segment-meta
    settings.synapseDeleteBorder = paredSettings["synapse_delete_border"].getFloat();
    settings.actionPotential = paredSettings["action_potential"].getFloat();
    settings.nodeCooldown = paredSettings["node_cooldown"].getFloat();
    settings.memorizing = paredSettings["memorizing"].getFloat();
    settings.gliaValue = paredSettings["glia_value"].getFloat();
    settings.maxSynapseWeight = paredSettings["max_synapse_weight"].getFloat();
    settings.refractionTime = paredSettings["refraction_time"].getInt();
    settings.signNeg = paredSettings["sign_neg"].getFloat();
    settings.potentialOverflow = paredSettings["potential_overflow"].getFloat();
    settings.multiplicatorRange = paredSettings["multiplicator_range"].getInt();

    const uint32_t numberOfNodeBricks = parsedContent.get("bricks").size();
    uint32_t totalNumberOfNodes = 0;
    for(uint32_t i = 0; i < numberOfNodeBricks; i++) {
        totalNumberOfNodes += parsedContent.get("bricks").get(i).get("number_of_nodes").getInt();
    }

    // init segment
    KyoukoRoot::m_networkCluster = new NetworkCluster();
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;
    cluster->initMetaData = newCluster.initMetaData;
    cluster->networkMetaData = newCluster.networkMetaData;

    DynamicSegment* newSegment = new DynamicSegment(numberOfNodeBricks,
                                                    totalNumberOfNodes,
                                                    newCluster.initMetaData.maxSynapseSections);  // TODO: correct number of outputs

    newSegment->segmentSettings[0] = settings;

    // fill array with empty nodes
    newSegment->addBricksToSegment(parsedContent);
    newSegment->initTargetBrickList();

    return true;
}

