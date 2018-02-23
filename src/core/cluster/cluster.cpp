#include <core/cluster/cluster.h>
#include <files//ioBuffer.h>

namespace KyoChan_Network
{

/**
 * @brief Cluster::Cluster
 * @param clusterId
 * @param clusterType
 * @param parentCluster
 */
Cluster::Cluster(quint32 clusterId,
                 ClusterType clusterType,
                 Cluster *parentCluster)
{
    m_clusterId = clusterId;
    m_clusterType = clusterType;
    m_parentCluster = parentCluster;

    if(m_parentCluster != nullptr) {
        m_clusterLevel = m_parentCluster->getClusterLevel() + 1;
    }
}

/**
 * @brief Cluster::~Cluster
 */
Cluster::~Cluster()
{

}

/**
 * @brief Cluster::getChildCluster
 * @param clusterId
 * @return
 */
Cluster *Cluster::getChildCluster(quint8 clusterId)
{
    if(clusterId > 7) {
        return nullptr;
    }
    return m_childClusterIds[clusterId];
}

/**
 * @brief Cluster::getParentCluster
 * @return
 */
Cluster *Cluster::getParentCluster()
{
    return m_parentCluster;
}

/**
 * @brief Cluster::getClusterId
 * @return
 */
quint32 Cluster::getClusterId() const
{
    return m_clusterId;
}

/**
 * @brief Cluster::getClusterType
 * @return
 */
ClusterType Cluster::getClusterType() const
{
    return m_clusterType;
}

/**
 * @brief Cluster::getClusterLevel
 * @return
 */
quint16 Cluster::getClusterLevel() const
{
    return m_clusterLevel;
}

/**
 * @brief Cluster::addNeighbor
 * @param clusterId
 * @param side
 * @return
 */
bool Cluster::addNeighbor(const quint8 side, const quint32 targetId)
{
    if(side > 6) {
        return false;
    }

    getMetaData()->neighors[side] = targetId;
    m_buffer->syncBlocks(0, 0);
    return true;
}

/**
 * @brief Cluster::getMetaData
 * @return
 */
KyoChanMetaData *Cluster::getMetaData()
{
    return (KyoChanMetaData*)m_buffer->getBlock(0);
}



}
