#include <core/cluster/clusterHandler.h>

namespace KyoChan_Network
{

ClusterManager::ClusterManager()
{

}

bool ClusterManager::addCluster(const ClusterID clusterId, Cluster *cluster)
{
    if(m_allClusters.contains(clusterId)) {
        return false;
    }
    m_allClusters.insert(clusterId, cluster);
    return true;
}

Cluster *ClusterManager::getCluster(const ClusterID clusterId)
{
    return  m_allClusters.value(clusterId, nullptr);
}

}
