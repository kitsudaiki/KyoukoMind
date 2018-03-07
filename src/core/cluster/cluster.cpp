#include <core/cluster/cluster.h>
#include <files//ioBuffer.h>

namespace KyoukoMind
{

/**
 * @brief Cluster::Cluster
 * @param clusterId
 * @param clusterType
 * @param directoryPath
 */
Cluster::Cluster(ClusterID clusterId,
                 ClusterType clusterType,
                 const QString directoryPath)
{
    m_clusterId = clusterId;
    m_clusterType = clusterType;

    initFile(clusterId, directoryPath);
    m_buffer->allocateBlocks(1);
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
bool Cluster::addNeighbor(const quint8 side, const Neighbor target)
{
    if(side > 8) {
        return false;
    }
    m_metaData.neighors[side] = target;
    updateMetaData(m_metaData);
    return true;
}

/**
 * @brief Cluster::updateMetaData
 * @param metaData
 */
void Cluster::updateMetaData(ClusterMetaData metaData)
{
    int size = sizeof(ClusterMetaData);
    void* metaDataBlock = m_buffer->getBlock(0);
    memcpy(metaDataBlock, &metaData, size);
    m_buffer->syncBlocks(0, 0);
}

/**
 * @brief Cluster::getMetaData
 */
void Cluster::getMetaData()
{
    ClusterMetaData* metaDataPointer = (ClusterMetaData*)m_buffer->getBlock(0);
    m_metaData = *metaDataPointer;
}

/**
 * @brief Cluster::convertId
 * @param clusterId
 * @return
 */
ClusterID Cluster::convertId(const quint32 clusterId)
{

}

/**
 * @brief Cluster::initFile
 * @param clusterId
 * @param directoryPath
 */
void Cluster::initFile(const ClusterID clusterId,
                       const QString directoryPath)
{
    QString filePath = directoryPath
                     + "/cluster_" + QString::number(clusterId.x)
                             + "_" + QString::number(clusterId.y)
                             + "_" + QString::number(clusterId.z);
    m_buffer = new Persistence::IOBuffer(filePath);
}


}
