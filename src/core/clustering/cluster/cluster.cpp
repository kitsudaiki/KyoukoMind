/**
 *  @file    cluster.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/clustering/cluster/cluster.h>
#include <files/dataBuffer.h>
#include <core/messaging/messageController.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

namespace KyoukoMind
{

/**
 * @brief Cluster::Cluster
 * @param clusterId
 * @param clusterType
 * @param directoryPath
 * @param controller
 */
Cluster::Cluster(const ClusterID &clusterId,
                 const uint8_t clusterType,
                 const std::string directoryPath,
                 MessageController *controller)
{
    m_metaData.clusterId = clusterId;
    initMessageBuffer(controller);

    if(clusterType > 3) {
        m_metaData.clusterType = EMPTY_CLUSTER;
    } else {
        m_metaData.clusterType = clusterType;
    }

    initFile(clusterId, directoryPath);
    m_clusterDataBuffer->allocateBlocks(1);
    updateMetaData(m_metaData);
}

/**
 * @brief Cluster::~Cluster
 */
Cluster::~Cluster()
{
    if(m_clusterDataBuffer != nullptr) {
        m_clusterDataBuffer->closeBuffer();
        delete m_clusterDataBuffer;
        m_clusterDataBuffer = nullptr;
    }
}

/**
 * @brief Cluster::getClusterId
 * @return
 */
ClusterID Cluster::getClusterId() const
{
    return m_metaData.clusterId;
}

/**
 * @brief Cluster::getClusterType
 * @return
 */
uint8_t Cluster::getClusterType() const
{
    return m_metaData.clusterType;
}

/**
 * @brief Cluster::addNeighbor
 * @param clusterId
 * @param side
 * @return
 */
bool Cluster::addNeighbor(const uint8_t side, const Neighbor target)
{
    if(side > 16) {
        return false;
    }
    if(side == 15) {
        m_metaData.outgoing = 1;
    }
    if(side == 0) {
        m_metaData.incoming = 1;
    }
    m_metaData.neighors[side] = target;
    updateMetaData(m_metaData);
    return true;
}

/**
 * @brief Cluster::getNeighbors
 * @return
 */
Neighbor *Cluster::getNeighbors()
{
    return &m_metaData.neighors[0];
}

/**
 * @brief Cluster::getNeighborId
 * @param side
 * @return
 */
ClusterID Cluster::getNeighborId(const uint8_t side)
{
    m_metaData.neighors[side].targetClusterId;
}

/**
 * @brief Cluster::getNextNewEdgeId
 * @return
 */
uint32_t Cluster::getNextNewEdgeId()
{
    m_newEdgeIdCounter++;
    return m_newEdgeIdCounter;
}

/**
 * @brief Cluster::updateMetaData
 * @param metaData
 */
void Cluster::updateMetaData(ClusterMetaData metaData)
{
    uint32_t size = sizeof(ClusterMetaData);
    void* metaDataBlock = m_clusterDataBuffer->getBlock(0);
    memcpy(metaDataBlock, &metaData, size);
    m_clusterDataBuffer->syncBlocks(0, 0);
}

/**
 * @brief Cluster::getMetaData
 */
void Cluster::getMetaData()
{
    ClusterMetaData* metaDataPointer = (ClusterMetaData*)m_clusterDataBuffer->getBlock(0);
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
    m_clusterDataBuffer = new PerformanceIO::DataBuffer(filePath);
}

/**
 * @brief Cluster::isReady
 * @return
 */
bool Cluster::isReady() const
{
    return m_incomingMessageQueue->isReady();
}

/**
 * @brief Cluster::getIncomingMessageBuffer
 * @return
 */
IncomingMessageBuffer *Cluster::getIncomingMessageBuffer() const
{
    return m_incomingMessageQueue;
}

/**
 * @brief Cluster::getOutgoingMessageBuffer
 * @return
 */
OutgoingMessageBuffer *Cluster::getOutgoingMessageBuffer() const
{
    return m_outgoingMessageQueue;
}

/**
 * @brief Cluster::initMessageBuffer
 * @param clusterId
 * @param controller
 */
void Cluster::initMessageBuffer(MessageController *controller)
{
    m_incomingMessageQueue = new IncomingMessageBuffer(this, controller);
    m_outgoingMessageQueue = new OutgoingMessageBuffer(this, controller);
}

}
