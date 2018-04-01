#include <core/networkManager.h>
#include <settings/config.h>
#include <KyoChanNetwork.h>

#include <core/cluster/clusterHandler.h>
#include <core/processing/processingUnitHandler.h>
#include <core/messaging/messagecontroller.h>
#include <core/networkInitializer.h>

#include <core/cluster/cluster.h>
#include <core/cluster/emptyCluster.h>
#include <core/cluster/edgeCluster.h>
#include <core/cluster/nodeCluster.h>

namespace KyoukoMind
{

/**
 * @brief NetManager::NetManager
 */
NetworkManager::NetworkManager()
{
    m_clusterManager = new ClusterHandler();
    m_processingUnitHandler = new ProcessingUnitHandler();
    m_messageController = new MessageController();

    bool ok = false;
    std::string initialFile = KyoukoNetwork::m_config->getInitialFilePath(&ok);
    std::string directoryPath = KyoukoNetwork::m_config->getDirectoryPath(&ok);

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
        NetworkInitializer init(initialFile,
                                directoryPath,
                                m_clusterManager,
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

/**
 * @brief NetworkManager::getMindDimension
 * @return
 */
uint32_t *NetworkManager::getMindDimension()
{
    return m_mindDimensions;
}



}
