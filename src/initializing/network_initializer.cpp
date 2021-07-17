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
#include <initializing/brick_initializer.h>

#include <core/processing/processing_unit_handler.h>
#include <core/processing/gpu/gpu_processing_uint.h>

#include <libKitsunemimiAiParser/ai_parser_input.h>
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
    m_brickInitializer = new BrickInitializer();
}

/**
 * @brief initialize new network
 *
 * @return true, if successfull, else false
 */
bool
ClusterInitializer::initNetwork(const std::string &initialFile,
                                const std::string &configFile)
{
    bool success = false;

    LOG_INFO("no files found. Try to create a new cluster");

    LOG_INFO("use init-file: " + initialFile);

    std::string initFileContent = "";
    std::string errorMessage = "";
    if(Kitsunemimi::Persistence::readFile(initFileContent, initialFile, errorMessage) == false)
    {
        LOG_ERROR(errorMessage);
        return false;
    }

    LOG_INFO("use init-file: " + configFile);

    std::string configFileContent = "";
    if(Kitsunemimi::Persistence::readFile(configFileContent, configFile, errorMessage) == false)
    {
        LOG_ERROR(errorMessage);
        return false;
    }

    success = createNewNetwork(initFileContent, configFileContent);
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
ClusterInitializer::createNewNetwork(const std::string &fileContent,
                                     const std::string &configFileContent)
{
    // init randomizer
    srand(time(NULL));

    // check if values are valid
    if(fileContent == "") {
        return false;
    }

    // parse input
    std::string errorMessage = "";
    Kitsunemimi::Ai::AiBaseMeta parsedContent;
    Kitsunemimi::Ai::AiParserInput inputParser;
    const bool ret = inputParser.parseAi(parsedContent, fileContent, configFileContent, errorMessage);
    if(ret == false)
    {
        LOG_ERROR("error while parsing input: " + errorMessage);
        return false;
    }

    // init segment
    const uint32_t numberOfNodeBricks = parsedContent.numberOfNodeBricks;
    const uint32_t totalNumberOfNodes = numberOfNodeBricks * parsedContent.initMetaData.nodesPerBrick;

    // init segment
    KyoukoRoot::m_networkCluster = new NetworkCluster();
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;
    cluster->initMetaData = parsedContent.initMetaData;
    cluster->networkMetaData = parsedContent.networkMetaData;

    // init predefinde random-values
    const uint32_t numberOfRandValues = 10*1024*1024;
    cluster->randomValues = new uint32_t[numberOfRandValues];
    for(uint32_t i = 0; i < numberOfRandValues; i++) {
        cluster->randomValues[i] = static_cast<uint32_t>(rand());
    }

    cluster->synapseSegment = initSynapseSegment(parsedContent.numberOfNodeBricks,
                                                 totalNumberOfNodes,
                                                 parsedContent.initMetaData.maxSynapseSections,
                                                 (totalNumberOfNodes / parsedContent.numberOfNodeBricks) * parsedContent.numberOfInputBricks,
                                                 10 * parsedContent.numberOfOutputBricks,
                                                 numberOfRandValues);

    cluster->synapseSegment->synapseSettings[0] = parsedContent.synapseMetaData;

    // fill array with empty nodes
    initializeNodes(*cluster->synapseSegment,
                    &cluster->initMetaData);
    addBricksToSegment(*cluster->synapseSegment,
                       &cluster->initMetaData,
                       parsedContent);
    m_brickInitializer->initTargetBrickList(*cluster->synapseSegment,
                                            &cluster->initMetaData);
  //  m_brickInitializer->initializeAxons(*cluster->synapseSegment);

    return true;
}
