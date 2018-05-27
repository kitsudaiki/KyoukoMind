/**
 *  @file    netowrkManager.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/networkManager.h>
#include <settings/config.h>
#include <kyochanNetwork.h>

#include <core/clustering/clusterHandler.h>
#include <core/processing/processingUnitHandler.h>
#include <core/messaging/messageController.h>
#include <core/initializing/networkInitializer.h>

#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/structs/clusterMeta.h>

namespace KyoukoMind
{

/**
 * @brief NetManager::NetManager
 */
NetworkManager::NetworkManager()
{
    assert(sizeof(ClusterMetaData) < 4096);

    m_clusterHandler = new ClusterHandler();
    m_processingUnitHandler = new ProcessingUnitHandler(m_clusterHandler);
    m_messageController = new MessageController();

    initNetwork();
}

/**
 * @brief NetworkManager::getMessageController
 * @return
 */
MessageController *NetworkManager::getMessageController() const
{
    return m_messageController;
}

/**
 * @brief NetworkManager::getClusterHandler
 * @return
 */
ClusterHandler *NetworkManager::getClusterHandler() const
{
    return m_clusterHandler;
}

/**
 * @brief NetworkManager::run
 */
void NetworkManager::run()
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
void NetworkManager::initNetwork()
{
    bool ok = false;
    std::string initialFile = KyoukoNetwork::m_config->getInitialFilePath(&ok);
    std::string directoryPath = KyoukoNetwork::m_config->getDirectoryPath(&ok);

    m_processingUnitHandler->initProcessingUnits(NUMBER_OF_PROCESSING_UNITS);

    std::vector<std::string> clusterFiles;
    // get all files in the directory
    DIR *dir;
    struct dirent *ent;
    if((dir = opendir(directoryPath.c_str())) != nullptr) {
        while((ent = readdir(dir)) != nullptr) {
            std::string tempFileName = ent->d_name;
            if(tempFileName.at(0) != '.') {
                clusterFiles.push_back(ent->d_name);
            }
        }
        closedir (dir);
    }

    if(clusterFiles.size() == 0)
    {
        // read into string
        std::ifstream inFile;
        inFile.open(initialFile);
        std::stringstream strStream;
        strStream << inFile.rdbuf();
        std::string string_content = strStream.str();

        NetworkInitializer init(string_content,
                                directoryPath,
                                m_clusterHandler,
                                m_messageController);
        bool successfulInit = init.initNetwork();
        assert(successfulInit);
    }
    else {
        for(uint32_t i = 0; i < clusterFiles.size(); i++) {
            // TODO
        }
    }
}

}
