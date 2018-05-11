/**
 *  @file    clusterHandler.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/clustering/clusterHandler.h>
#include <core/clustering/clusterQueue.h>
#include <core/clustering/cluster/edgeCluster.h>

namespace KyoukoMind
{

/**
 * @brief ClusterManager::ClusterManager
 */
ClusterHandler::ClusterHandler()
{
    m_clusterQueue = new ClusterQueue();
}

/**
 * @brief ClusterManager::~ClusterManager
 */
ClusterHandler::~ClusterHandler()
{
    delete m_clusterQueue;
    clearAllCluster();
}

/**
 * @brief ClusterManager::addCluster
 * @param clusterId
 * @param cluster
 * @return
 */
bool ClusterHandler::addCluster(const ClusterID clusterId, EdgeCluster* cluster)
{
    if(m_allClusters.find(clusterId) != m_allClusters.end()) {
        return false;
    }
    m_allClusters.insert(std::pair<ClusterID, EdgeCluster*>(clusterId, cluster));
    m_clusterQueue->addCluster(cluster);
    return true;
}

/**
 * @brief ClusterManager::getCluster
 * @param clusterId
 * @return
 */
EdgeCluster *ClusterHandler::getCluster(const ClusterID clusterId)
{
    std::map<ClusterID, EdgeCluster*>::iterator it;
    it = m_allClusters.find(clusterId);
    if(it != m_allClusters.end()) {
        return it->second;
    }
    return nullptr;
}

/**
 * @brief ClusterManager::deleteCluster
 * @param clusterId
 * @return
 */
bool ClusterHandler::deleteCluster(const ClusterID clusterId)
{
    std::map<ClusterID, EdgeCluster*>::iterator it;
    it = m_allClusters.find(clusterId);
    if(it != m_allClusters.end()) {
        m_allClusters.erase(it);
        return true;
    }
    return false;
}

/**
 * @brief ClusterManager::clearAllCluster
 */
void ClusterHandler::clearAllCluster()
{
    std::map<ClusterID, EdgeCluster*>::iterator it;
    for(it = m_allClusters.begin(); it != m_allClusters.end(); ++it) {
        EdgeCluster* tempCluster = it->second;
        delete tempCluster;
    }
    m_allClusters.clear();
    m_clusterQueue->clearQueue();
}

/**
 * @brief ClusterHandler::getClusterQueue
 * @return
 */
ClusterQueue *ClusterHandler::getClusterQueue() const
{
    return m_clusterQueue;
}

}
