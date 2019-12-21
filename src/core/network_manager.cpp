/**
 *  @file    network_manager.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/network_manager.h>
#include <settings/config.h>
#include <kyouko_network.h>

#include <core/bricks/brick_handler.h>
#include <core/processing/processing_unit_handler.h>

#include <core/bricks/brick_objects/brick.h>

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

    std::string initialFile = KyoukoNetwork::m_config->getInitialFilePath();
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
void
NetworkManager::initNetwork()
{
    std::string initialFile = KyoukoNetwork::m_config->getInitialFilePath();
    std::string directoryPath = KyoukoNetwork::m_config->getDirectoryPath();

    m_processingUnitHandler->initProcessingUnits(NUMBER_OF_PROCESSING_UNITS);

    std::vector<std::string> brickFiles;
    // get all files in the directory
    DIR *dir;
    struct dirent *ent;
    if((dir = opendir(directoryPath.c_str())) != nullptr) {
        while((ent = readdir(dir)) != nullptr) {
            std::string tempFileName = ent->d_name;
            if(tempFileName.at(0) != '.') {
                brickFiles.push_back(ent->d_name);
            }
        }
        closedir (dir);
    }

    if(brickFiles.size() == 0)
    {
        std::string fileContent = readFile(initialFile);
        createNewNetwork(fileContent);
    }
    else {
        for(uint32_t i = 0; i < brickFiles.size(); i++) {
            // TODO
        }
    }
}

} // namespace KyoukoMind
