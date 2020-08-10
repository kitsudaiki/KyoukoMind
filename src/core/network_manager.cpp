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

#include <core/object_handling/brick.h>

#include <initializing/file_parser.h>
#include <initializing/network_initializer.h>

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
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;

    while(!m_abort)
    {
        if(m_block) {
            blockThread();
        }

        usleep(PROCESS_INTERVAL);
        start = std::chrono::system_clock::now();
        m_phase1->triggerBarrier();
        m_phase2->triggerBarrier();
        m_phase3->triggerBarrier();
        end = std::chrono::system_clock::now();
        const float gpu0 = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        LOG_WARNING("cycle-time: " + std::to_string(gpu0 / 1000.0f) + '\n');
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
