#include <core/cluster/cluster.h>
#include <files//ioBuffer.h>

namespace KyoChan_Network
{

/**
 * @brief Cluster::Cluster
 * @param clusterId
 * @param clusterType
 */
Cluster::Cluster(ClusterID clusterId,
                 ClusterType clusterType)
{
    m_clusterId = clusterId;
    m_clusterType = clusterType;
}

/**
 * @brief Cluster::~Cluster
 */
Cluster::~Cluster()
{

}

/**
 * @brief Cluster::getClusterId
 * @return
 */
ClusterID Cluster::getClusterId() const
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

    //getMetaData()->neighors[side] = targetId;
    //m_buffer->syncBlocks(0, 0);
    return true;
}

/**
 * @brief Cluster::getMetaData
 * @return
 */
CluserMetaData *Cluster::getMetaData()
{
    return (CluserMetaData*)m_buffer->getBlock(0);
}



}
