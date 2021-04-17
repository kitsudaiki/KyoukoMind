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
#include <core/objects/global_values.h>

#include <initializing/layered_brick_initializier.h>
#include <initializing/random_brick_initializer.h>

#include <core/processing/processing_unit_handler.h>
#include <core/processing/gpu/gpu_processing_uint.h>

#include <libKitsunemimiPersistence/logger/logger.h>

#include <libKitsunemimiAiParser/ai_parser_input.h>

#include <initializing/segment_initailzing.h>

/**
 * @brief constructor
 */
NetworkInitializer::NetworkInitializer()
{
    m_brickInitializer = new LayeredBrickInitializier();
}

/**
 * @brief create blank new network
 *
 * @param fileContent file to parse with the basic structure of the network
 *
 * @return true, if successfull, else false
 */
bool
NetworkInitializer::createNewNetwork(const std::string &fileContent,
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

    const uint32_t numberOfBricks = static_cast<uint32_t>(parsedContent.bricks.size());

    GlobalValues globalValues;
    globalValues.nodesPerBrick = parsedContent.initializingMeta.nodesPerBrick;
    globalValues.maxBrickDistance = parsedContent.initializingMeta.maxBrickDistance;
    globalValues.nodeLowerBorder = parsedContent.initializingMeta.nodeLowerBorder;
    globalValues.nodeUpperBorder = parsedContent.initializingMeta.nodeUpperBorder;
    globalValues.maxSynapseSections = parsedContent.initializingMeta.maxSynapseSections;
    globalValues.layer = parsedContent.initializingMeta.layer;

    globalValues.synapseDeleteBorder = parsedContent.processingMeta.synapseDeleteBorder;
    globalValues.actionPotential = parsedContent.processingMeta.actionPotential;
    globalValues.nodeCooldown = parsedContent.processingMeta.nodeCooldown;
    globalValues.memorizing = parsedContent.processingMeta.memorizing;
    globalValues.gliaValue = parsedContent.processingMeta.gliaValue;
    globalValues.maxSynapseWeight = parsedContent.processingMeta.maxSynapseWeight;
    globalValues.refractionTime = parsedContent.processingMeta.refractionTime;
    globalValues.cycleTime = parsedContent.processingMeta.cycleTime;
    globalValues.inputFlowGradiant = parsedContent.processingMeta.inputFlowGradiant;
    globalValues.nodeFlowGradiant = parsedContent.processingMeta.nodeFlowGradiant;
    globalValues.signNeg = parsedContent.processingMeta.signNeg;
    globalValues.potentialOverflow = parsedContent.processingMeta.potentialOverflow;


    // update message for the monitoring
    KyoukoRoot::monitoringBrickMessage.numberOfInfos = numberOfBricks;

    // init segment
    const uint32_t numberOfNodeBricks = parsedContent.numberOfNodeBricks;
    const uint32_t totalNumberOfNodes = numberOfNodeBricks * globalValues.nodesPerBrick;

    // init segment
    KyoukoRoot::m_synapseSegment = initSynapseSegment(parsedContent.numberOfNodeBricks,
                                                      totalNumberOfNodes,
                                                      globalValues.maxSynapseSections,
                                                      totalNumberOfNodes / parsedContent.numberOfNodeBricks,
                                                      1000);
    KyoukoRoot::m_outputSegment = initOutputSegment(10,
                                                    totalNumberOfNodes / parsedContent.numberOfNodeBricks,
                                                    1000);

    KyoukoRoot::m_synapseSegment->globalValues[0] = globalValues;
    KyoukoRoot::m_outputSegment->globalValues[0] = globalValues;

    // fill array with empty nodes
    initializeNodes(*KyoukoRoot::m_synapseSegment);
    addBricksToSegment(*KyoukoRoot::m_synapseSegment, parsedContent);
    m_brickInitializer->initTargetBrickList(*KyoukoRoot::m_synapseSegment);
    m_brickInitializer->initializeAxons(*KyoukoRoot::m_synapseSegment);

    return true;
}
