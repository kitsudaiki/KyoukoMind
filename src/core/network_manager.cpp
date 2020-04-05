/**
 *  @file    network_manager.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/network_manager.h>
#include <root_object.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiPersistence/files/file_methods.h>

#include <core/brick_handler.h>
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

    //m_trigger = new KyoukoMind::ConnectionTrigger();
    //KyoukoNetwork::m_mindClient->addNetworkTrigger(m_trigger);
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
    if(brickFiles.size() == 0)
    {
        LOG_INFO("no files found. Try to create a new cluster");
        const std::string initialFile = GET_STRING_CONFIG("Init", "file", success);
        if(success == false)
        {
            LOG_ERROR("no init-file set in the config-file");
            return false;
        }
        LOG_INFO("use init-file: " + initialFile);

        const std::string fileContent = readFile(initialFile);
        return createNewNetwork(fileContent);
    }
    else {
        for(uint32_t i = 0; i < brickFiles.size(); i++) {
            // TODO
        }
    }

    return true;
}

} // namespace KyoukoMind
