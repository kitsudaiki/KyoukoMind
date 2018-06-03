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
#include <core/clustering/cluster/cluster.h>

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
bool ClusterHandler::addCluster(const ClusterID clusterId, Cluster* cluster)
{
    if(m_allClusters.find(clusterId) != m_allClusters.end()) {
        return false;
    }
    m_allClusters.insert(std::pair<ClusterID, Cluster*>(clusterId, cluster));
    m_clusterQueue->addCluster(cluster);
    return true;
}

/**
 * @brief ClusterManager::getCluster
 * @param clusterId
 * @return
 */
Cluster *ClusterHandler::getCluster(const ClusterID clusterId)
{
    std::map<ClusterID, Cluster*>::iterator it;
    it = m_allClusters.find(clusterId);
    if(it != m_allClusters.end()) {
        return it->second;
    }
    return nullptr;
}

/**
 * @brief ClusterHandler::getNumberOfCluster
 * @return
 */
uint32_t ClusterHandler::getNumberOfCluster() const
{
    return m_allClusters.size();
}

/**
 * @brief ClusterManager::deleteCluster
 * @param clusterId
 * @return
 */
bool ClusterHandler::deleteCluster(const ClusterID clusterId)
{
    std::map<ClusterID, Cluster*>::iterator it;
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
    std::map<ClusterID, Cluster*>::iterator it;
    for(it = m_allClusters.begin(); it != m_allClusters.end(); ++it) {
        Cluster* tempCluster = it->second;
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
