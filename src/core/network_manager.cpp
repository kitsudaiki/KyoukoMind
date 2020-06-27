/**
 *  @file    network_manager.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#include <core/network_manager.h>
#include <kyouko_root.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiPersistence/files/file_methods.h>
#include <libKitsunemimiPersistence/files/text_file.h>

#include <core/processing/processing_unit_handler.h>

#include <core/objects/brick.h>

#include <initializing/file_parser.h>
#include <initializing/network_initializer.h>

namespace KyoukoMind
{

/**
 * @brief NetManager::NetManager
 */
NetworkManager::NetworkManager()
{
    assert(sizeof(Brick) < 4096);

    m_processingUnitHandler = new ProcessingUnitHandler();
    initNetwork();
}

/**
 * @brief NetworkManager::run
 */
void
NetworkManager::run()
{
    while(!m_abort)
    {
        if(m_block) {
            blockThread();
        }

        usleep(PROCESS_INTERVAL);
        m_processingUnitHandler->initNextCycle();
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

    m_processingUnitHandler->initProcessingUnits(NUMBER_OF_PROCESSING_UNITS);

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
        return initializer.createNewNetwork(fileContent);
    }
    else {
        for(uint32_t i = 0; i < brickFiles.size(); i++) {
            // TODO
        }
    }

    return true;
}

} // namespace KyoukoMind
