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
#include <core/client_handler.h>
#include <core/object_handling/brick.h>

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

    while(!m_abort)
    {
        if(m_block) {
            blockThread();
        }

        usleep(PROCESS_INTERVAL);
        m_phase1->triggerBarrier();
        edgeStart = std::chrono::system_clock::now();

        m_phase2->triggerBarrier();
        edgeEnd = std::chrono::system_clock::now();
        synapseStart = std::chrono::system_clock::now();

        m_phase3->triggerBarrier();
        synapseEnd = std::chrono::system_clock::now();

        const float edgeTime = duration_cast<chronoNanoSec>(edgeEnd - edgeStart).count();
        const float synapseTime = duration_cast<chronoNanoSec>(synapseEnd - synapseStart).count();

        KyoukoRoot::monitoringMetaMessage.timeEdgePhase = edgeTime;
        KyoukoRoot::monitoringMetaMessage.timeSynapsePhase = synapseTime;
        KyoukoRoot::monitoringMetaMessage.timeTotal = edgeTime + synapseTime;

        const std::string meta = KyoukoRoot::m_root->monitoringMetaMessage.toString();
        KyoukoRoot::m_clientHandler->sendToMonitoring(meta.c_str(), meta.size());

        // debug-output
        const std::string testClient = "poi";
        KyoukoRoot::m_clientHandler->sendToClient(testClient.c_str(), testClient.size());
        m_testMonitoring = "{\"bricks\": [";
        for(int i = 0; i < 20; i++)
        {
            for(int j = 0; j < 20; j++)
            {
                if(i == 0 && j == 0) {
                    m_testMonitoring += "";
                } else {
                    m_testMonitoring += ",";
                }

                const int rand = std::rand() % 400;
                const std::string part = "[" + std::to_string(i)
                                       + "," + std::to_string(j)
                                       + "," + std::to_string(rand) + "]";
                m_testMonitoring += part;
            }
        }
        m_testMonitoring += "]}";
        KyoukoRoot::m_clientHandler->sendToMonitoring(m_testMonitoring.c_str(),
                                                      m_testMonitoring.size());
    }
}

/**
 * @brief NetworkManager::initNetwork
 */
bool
NetworkManager::initNetwork()
{
    bool success = false;
    const std::string directoryPath = GET_STRING_CONFIG("Storage", "path", success);
    LOG_INFO("use storage-directory: " + directoryPath);

    std::vector<std::string> brickFiles;
    Kitsunemimi::Persistence::listFiles(brickFiles, directoryPath, false);

    if(brickFiles.size() == 0
            || bfs::exists(directoryPath) == false)
    {
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
    }
    else
    {
        for(uint32_t i = 0; i < brickFiles.size(); i++) {
            // TODO
        }
    }

    m_processingUnitHandler->initProcessingUnits(m_phase1,
                                                 m_phase2,
                                                 m_phase3,
                                                 NUMBER_OF_PROCESSING_UNITS);

    return true;
}
