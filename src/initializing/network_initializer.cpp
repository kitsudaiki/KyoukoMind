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

#include <core/objects/segment.h>
#include <core/objects/node.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

#include <initializing/segment_initailzing.h>
#include <initializing/segment_creation.h>

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

    // network-meta
    newCluster.networkMetaData.cycleTime = parsedContent.get("settings").get("cycle_time").getLong();

    // init-meta
    newCluster.initMetaData.nodesPerBrick = parsedContent.get("settings").get("nodes_per_brick").getInt();
    newCluster.initMetaData.nodeLowerBorder = parsedContent.get("settings").get("node_lower_border").getFloat();
    newCluster.initMetaData.nodeUpperBorder = parsedContent.get("settings").get("node_upper_border").getFloat();
    newCluster.initMetaData.maxBrickDistance = parsedContent.get("settings").get("max_brick_distance").getInt();
    newCluster.initMetaData.maxSynapseSections = parsedContent.get("settings").get("max_synapse_sections").getLong();
    newCluster.initMetaData.layer = parsedContent.get("settings").get("layer").getInt();

    // segment-meta
    settings.synapseDeleteBorder = parsedContent.get("settings").get("synapse_delete_border").getFloat();
    settings.actionPotential = parsedContent.get("settings").get("action_potential").getFloat();
    settings.nodeCooldown = parsedContent.get("settings").get("node_cooldown").getFloat();
    settings.memorizing = parsedContent.get("settings").get("memorizing").getFloat();
    settings.gliaValue = parsedContent.get("settings").get("glia_value").getFloat();
    settings.maxSynapseWeight = parsedContent.get("settings").get("max_synapse_weight").getFloat();
    settings.refractionTime = parsedContent.get("settings").get("refraction_time").getInt();
    settings.signNeg = parsedContent.get("settings").get("sign_neg").getFloat();
    settings.potentialOverflow = parsedContent.get("settings").get("potential_overflow").getFloat();
    settings.multiplicatorRange = parsedContent.get("settings").get("multiplicator_range").getInt();

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

    // init predefinde random-values
    const uint32_t numberOfRandValues = 10*1024*1024;
    cluster->randomValues = new uint32_t[numberOfRandValues];
    for(uint32_t i = 0; i < numberOfRandValues; i++) {
        cluster->randomValues[i] = static_cast<uint32_t>(rand());
    }

    cluster->synapseSegment = createNewSegment(numberOfNodeBricks,
                                               totalNumberOfNodes,
                                               newCluster.initMetaData.maxSynapseSections,
                                               784,  // TODO: correct number of inputs
                                               10,  // TODO: correct number of outputs
                                               numberOfRandValues);

    cluster->synapseSegment->synapseSettings[0] = settings;

    // fill array with empty nodes
    initializeNodes(*cluster->synapseSegment,
                    &cluster->initMetaData);
    addBricksToSegment(*cluster->synapseSegment,
                       &cluster->initMetaData,
                       parsedContent);
    initTargetBrickList(*cluster->synapseSegment,
                        &cluster->initMetaData);
  //  m_brickInitializer->initializeAxons(*cluster->synapseSegment);

    return true;
}

