/**
 * @file        network_manager.cpp
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

#include <core/network_manager.h>
#include <kyouko_root.h>

#include <libKitsunemimiCommon/threading/barrier.h>

#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiPersistence/files/file_methods.h>
#include <libKitsunemimiPersistence/files/text_file.h>

#include <core/processing/processing_unit_handler.h>
#include <core/connection_handler/client_connection_handler.h>
#include <core/connection_handler/monitoring_connection_handler.h>
#include <core/objects/brick.h>
#include <core/objects/output.h>
#include <core/objects/segment.h>
#include <core/objects/network_cluster.h>

#include <initializing/network_initializer.h>

#include <libKitsunemimiAiCommon/metadata.h>

using std::chrono::duration_cast;

/**
 * @brief NetManager::NetManager
 */
NetworkManager::NetworkManager()
{
    m_phase2 = new Kitsunemimi::Barrier(2);
    m_phase3 = new Kitsunemimi::Barrier(2);

    m_processingUnitHandler = new ProcessingUnitHandler();

    initNetwork();
}

uint32_t
NetworkManager::executeStep()
{
    // run phases of processing
    m_edgeStart = std::chrono::system_clock::now();

    m_phase2->triggerBarrier();
    m_edgeEnd = std::chrono::system_clock::now();
    m_synapseStart = std::chrono::system_clock::now();

    m_phase3->triggerBarrier();
    m_synapseEnd = std::chrono::system_clock::now();


    // calculate times
    const float edgeTime = duration_cast<chronoNanoSec>(m_edgeEnd - m_edgeStart).count();
    const float synapseTime = duration_cast<chronoNanoSec>(m_synapseEnd - m_synapseStart).count();
    const uint32_t usedTime = static_cast<uint32_t>((edgeTime + synapseTime) / 1024.0f);

    return usedTime;
}

/**
 * @brief NetworkManager::run
 */
void
NetworkManager::run()
{
    std::string errorMessage = "";

    uint32_t time = KyoukoRoot::m_networkCluster->networkMetaData.cycleTime;
    while(!m_abort)
    {
        if(m_block) {
            blockThread();
        }

        usleep(time);

        // handle learning
        KyoukoRoot::m_networkCluster->networkMetaData.lerningValue  = 0.0f;
        if(KyoukoRoot::m_freezeState)
        {
            KyoukoRoot::m_networkCluster->networkMetaData.lerningValue  = 1000.0f;
            KyoukoRoot::m_freezeState = false;
        }

        const uint32_t usedTime = executeStep();
        if(KyoukoRoot::m_networkCluster->networkMetaData.cycleTime > usedTime) {
            time = KyoukoRoot::m_networkCluster->networkMetaData.cycleTime - usedTime;
        } else {
            time = 1000;
        }

        KyoukoRoot::m_networkCluster->networkMetaData.lerningValue = 0.0f;
    }
}

/**
 * @brief initialize new network
 *
 * @return true, if successfull, else false
 */
bool
NetworkManager::initNetwork()
{
    bool success = false;

    LOG_INFO("no files found. Try to create a new cluster");

    const std::string initialFile = GET_STRING_CONFIG("Init", "file", success);
    if(success == false)
    {
        LOG_ERROR("no init-file set in the config-file");
        return false;
    }
    LOG_INFO("use init-file: " + initialFile);

    std::string initFileContent = "";
    std::string errorMessage = "";
    if(Kitsunemimi::Persistence::readFile(initFileContent, initialFile, errorMessage) == false)
    {
        LOG_ERROR(errorMessage);
        return false;
    }

    const std::string configFile = GET_STRING_CONFIG("Init", "config", success);
    if(success == false)
    {
        LOG_ERROR("no init-file set in the config-file");
        return false;
    }
    LOG_INFO("use init-file: " + configFile);

    std::string configFileContent = "";
    if(Kitsunemimi::Persistence::readFile(configFileContent, configFile, errorMessage) == false)
    {
        LOG_ERROR(errorMessage);
        return false;
    }

    NetworkInitializer initializer;
    success = initializer.createNewNetwork(initFileContent, configFileContent);
    if(success == false)
    {
        LOG_ERROR("failed to initialize network");
        return false;
    }

    m_processingUnitHandler->initProcessingUnits(m_phase2,
                                                 m_phase3,
                                                 NUMBER_OF_PROCESSING_UNITS);

    return true;
}
