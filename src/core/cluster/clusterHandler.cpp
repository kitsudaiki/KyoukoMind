#include <core/cluster/clusterHandler.h>

namespace KyoChan_Network
{

ClusterManager::ClusterManager()
{

}

bool ClusterManager::addCluster(const quint32 clusterId, Cluster *cluster)
{
    if(m_allClusters.contains(clusterId)) {
        return false;
    }
    m_allClusters.insert(clusterId, cluster);
    return true;
}

Cluster *ClusterManager::getCluster(const quint32 clusterId)
{
    return  m_allClusters.value(clusterId, nullptr);
}

}
