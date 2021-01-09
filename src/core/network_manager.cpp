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

#include <core/processing/input_output_processing.h>
#include <core/processing/processing_unit_handler.h>
#include <core/connection_handler/client_connection_handler.h>
#include <core/connection_handler/monitoring_connection_handler.h>
#include <core/objects/brick.h>
#include <core/objects/segment.h>
#include <core/objects/global_values.h>

#include <initializing/network_initializer.h>

using std::chrono::duration_cast;

/**
 * @brief NetManager::NetManager
 */
NetworkManager::NetworkManager()
{
    m_phase1 = new Kitsunemimi::Barrier(3);
    m_phase2 = new Kitsunemimi::Barrier(3);
    m_phase3 = new Kitsunemimi::Barrier(3);

    m_processingUnitHandler = new ProcessingUnitHandler();

    initNetwork();
}

/**
 * @brief NetworkManager::run
 */
void
NetworkManager::run()
{
    GlobalValues* globalValues = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    std::chrono::high_resolution_clock::time_point edgeStart;
    std::chrono::high_resolution_clock::time_point edgeEnd;

    std::chrono::high_resolution_clock::time_point synapseStart;
    std::chrono::high_resolution_clock::time_point synapseEnd;

    uint32_t time = PROCESS_INTERVAL;
    while(!m_abort)
    {
        if(m_block) {
            blockThread();
        }

        usleep(time);

        // handle learning
        float newLearningValue = 0.0f;
        if(KyoukoRoot::m_freezeState)
        {
            newLearningValue = 0.2f;
            KyoukoRoot::m_freezeState = false;
        }
        globalValues->lerningValue = newLearningValue;

        KyoukoRoot::m_ioHandler->processInputMapping();
        KyoukoRoot::m_ioHandler->processOutputMapping();

        // run phases of processing
        m_phase1->triggerBarrier();

        edgeStart = std::chrono::system_clock::now();

        m_phase2->triggerBarrier();
        edgeEnd = std::chrono::system_clock::now();
        synapseStart = std::chrono::system_clock::now();

        m_phase3->triggerBarrier();
        synapseEnd = std::chrono::system_clock::now();


        // calculate times
        const float edgeTime = duration_cast<chronoNanoSec>(edgeEnd - edgeStart).count();
        const float synapseTime = duration_cast<chronoNanoSec>(synapseEnd - synapseStart).count();

        if(PROCESS_INTERVAL > static_cast<uint32_t>((edgeTime + synapseTime) / 1024.0f)) {
            time = PROCESS_INTERVAL - static_cast<uint32_t>((edgeTime + synapseTime) / 1024.0f);
        } else {
            time = 1000;
        }

        // total times
        KyoukoRoot::monitoringMetaMessage.edgePhase = edgeTime;
        KyoukoRoot::monitoringMetaMessage.synapsePhase = synapseTime;
        KyoukoRoot::monitoringMetaMessage.totalCycle = edgeTime + synapseTime;

        // object-numbers in item-buffer
        const uint64_t numberOfSynapseSections = KyoukoRoot::m_segment->synapses.numberOfItems;
        KyoukoRoot::monitoringMetaMessage.synapseSections = numberOfSynapseSections;
        KyoukoRoot::monitoringMetaMessage.nodes = KyoukoRoot::m_segment->nodes.numberOfItems;
        KyoukoRoot::monitoringMetaMessage.edgeSections = KyoukoRoot::m_segment->edges.numberOfItems;

        // monitoring-output
        const std::string meta = KyoukoRoot::m_root->monitoringMetaMessage.toString();
        KyoukoRoot::m_monitoringHandler->sendToMonitoring(meta.c_str(), meta.size());
        KyoukoRoot::m_monitoringHandler->sendToMonitoring();
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

    std::string fileContent = "";
    std::string errorMessage = "";
    if(Kitsunemimi::Persistence::readFile(fileContent, initialFile, errorMessage) == false)
    {
        LOG_ERROR(errorMessage);
        return false;
    }

    NetworkInitializer initializer;
    success = initializer.createNewNetwork(fileContent);
    if(success == false)
    {
        LOG_ERROR("failed to initialize network");
        return false;
    }

    m_processingUnitHandler->initProcessingUnits(m_phase1,
                                                 m_phase2,
                                                 m_phase3,
                                                 NUMBER_OF_PROCESSING_UNITS);

    return true;
}
