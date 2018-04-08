/**
 *  @file    cluster.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/cluster/cluster.h>
#include <files/ioBuffer.h>
#include <core/messaging/messageController.h>

namespace KyoukoMind
{

/**
 * @brief Cluster::Cluster
 * @param clusterId
 * @param directoryPath
 */
Cluster::Cluster(const ClusterID &clusterId,
                 const std::string directoryPath)
{
    m_clusterId = clusterId;

    initFile(clusterId, directoryPath);
    m_buffer->allocateBlocks(1);
}

/**
 * @brief Cluster::~Cluster
 */
Cluster::~Cluster()
{
    if(m_buffer != nullptr) {
        m_buffer->closeBuffer();
        delete m_buffer;
        m_buffer = nullptr;
    }
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
bool Cluster::addNeighbor(const uint8_t side, const Neighbor target)
{
    if(side > 9) {
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
 * @brief Cluster::initFile
 * @param clusterId
 * @param directoryPath
 */
void Cluster::initFile(const ClusterID clusterId,
                       const std::string directoryPath)
{
    std::string filePath = directoryPath
                         + "/cluster_" + std::to_string(clusterId);
    m_buffer = new PerformanceIO::IOBuffer(filePath);
}


}
