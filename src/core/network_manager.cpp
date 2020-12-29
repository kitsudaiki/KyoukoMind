/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
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
#include <core/objects/segment.h>
#include <core/objects/global_values.h>

#include <initializing/file_parser.h>
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
        calcNewLearningValue();

        // run phases of processing
        m_phase1->triggerBarrier();
        synapseStart = std::chrono::system_clock::now();

        m_phase2->triggerBarrier();
        synapseEnd = std::chrono::system_clock::now();
        edgeStart = std::chrono::system_clock::now();

        m_phase3->triggerBarrier();
        edgeEnd = std::chrono::system_clock::now();

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
 * @brief NetworkManager::calcNewLearningValue
 */
void
NetworkManager::calcNewLearningValue()
{
    float newLearningValue = 0.0f;
    GlobalValues* globalValues = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    Brick* brick = getBuffer<Brick>(KyoukoRoot::m_segment->bricks);

    m_actualOutput = brick[60].getOutputValues();
    m_should = brick[60].getShouldValues();
    if(m_should.size() == 0) {
        globalValues->lerningValue = newLearningValue;
        return;
    }

    float summedOutput = 0.0f;

    for(uint32_t j = 0; j < m_actualOutput.size(); j++) {
        summedOutput += m_actualOutput.at(j);
        //std::cout<<"output: "<<m_actualOutput.at(j)<<std::endl;
    }
    brick[60].resetOutputValues();
    summedOutput /= static_cast<float>(m_actualOutput.size());

    // make result smooth
    m_outBuffer[m_outBufferPos] = summedOutput;
    m_outBufferPos = (m_outBufferPos + 1) % 5;

    float result = 0.0f;
    for(uint32_t i = 0; i < 5; i++) {
        result += m_outBuffer[i];
    }
    result /= 5.0f;

    KyoukoRoot::m_clientHandler->sendToClient(std::to_string(result));
    LOG_WARNING("-----------------------------------------------");
    LOG_WARNING("output: " + std::to_string(result));

    if(KyoukoRoot::m_freezeState)
    {
        newLearningValue = 0.2f;
        KyoukoRoot::m_freezeState = false;
    }

    globalValues->lerningValue = newLearningValue;
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
