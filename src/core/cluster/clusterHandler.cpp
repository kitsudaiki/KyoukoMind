#include <core/cluster/clusterHandler.h>
#include <core/cluster/cluster.h>

namespace KyoukoMind
{

/**
 * @brief ClusterManager::ClusterManager
 */
ClusterHandler::ClusterHandler()
{

}

/**
 * @brief ClusterManager::~ClusterManager
 */
ClusterHandler::~ClusterHandler()
{
    clearAllCluster();
}

/**
 * @brief ClusterManager::addCluster
 * @param clusterId
 * @param cluster
 * @return
 */
bool ClusterHandler::addCluster(const ClusterID clusterId, Cluster *cluster)
{
    if(m_allClusters.contains(clusterId)) {
        return false;
    }
    m_allClusters.insert(clusterId, cluster);
    return true;
}

/**
 * @brief ClusterManager::getCluster
 * @param clusterId
 * @return
 */
Cluster *ClusterHandler::getCluster(const ClusterID clusterId)
{
    return m_allClusters.value(clusterId, nullptr);
}

/**
 * @brief ClusterManager::deleteCluster
 * @param clusterId
 * @return
 */
bool ClusterHandler::deleteCluster(const ClusterID clusterId)
{
    if(m_allClusters.contains(clusterId)) {
        return false;
    }
    Cluster* tempCluster = m_allClusters.value(clusterId);
    m_allClusters.remove(clusterId);
    delete tempCluster;
    return true;
}

/**
 * @brief ClusterManager::clearAllCluster
 */
void ClusterHandler::clearAllCluster()
{
    QMap<ClusterID, Cluster*>::iterator it;
    for (it = m_allClusters.begin(); it != m_allClusters.end(); ++it) {
        Cluster* tempCluster = it.value();
        delete tempCluster;
    }
    m_allClusters.clear();
}

}
