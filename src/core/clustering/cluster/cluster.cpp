#include "cluster.h"

#include <files/dataBuffer.h>
#include <core/messaging/messageController.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>
#include <kyochanNetwork.h>

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
    // creates a file for the cluster with buffer
    // TODO: readd file-path after tests
    std::string filePath = directoryPath
                         + "/cluster_" + std::to_string(clusterId);
    m_clusterDataBuffer = new PerformanceIO::DataBuffer("");

    // init the buffer and the file with the first block for the meta-data
    m_clusterDataBuffer->allocateBlocks(1);
    updateMetaData();

    // init empty neighbor-list to avoid not initialized values
    for(uint32_t i = 0; i < 17; i++) {
        Neighbor emptyNeighbor;
        emptyNeighbor.targetSide = 16 - i;
        m_metaData.neighors[i] = emptyNeighbor;
    }
}

/**
 * @brief Cluster::~Cluster
 */
Cluster::~Cluster()
{
    if(m_clusterDataBuffer != nullptr)
    {
        m_clusterDataBuffer->closeBuffer();
        delete m_clusterDataBuffer;
        m_clusterDataBuffer = nullptr;
    }
}


/**
 * @brief Cluster::getMetaData get the meta-data of the cluster
 * @return meta-data-object
 */
ClusterMetaData Cluster::getMetaData() const
{
    return m_metaData;
}

/**
 * @brief Cluster::getMetaDataFromBuffer reads the meta-data from the file-buffer
 */
void Cluster::getMetaDataFromBuffer()
{
    ClusterMetaData* metaDataPointer = (ClusterMetaData*)m_clusterDataBuffer->getBlock(0);
    m_metaData = *metaDataPointer;
}


/**
 * @brief Cluster::updateMetaData write the current mata-data to the buffer and the file
 */
void Cluster::updateMetaData()
{
    uint32_t size = sizeof(ClusterMetaData);
    void* metaDataBlock = m_clusterDataBuffer->getBlock(0);
    memcpy(metaDataBlock, &m_metaData, size);
    m_clusterDataBuffer->syncBlocks(0, 0);
}

/**
 * @brief Cluster::getClusterId returns the cluster-id from the meta-data
 * @return id of the cluster
 */
ClusterID Cluster::getClusterId() const
{
    return m_metaData.clusterId;
}

/**
 * @brief Cluster::getClusterType returns the cluster-type from the meta-data
 * @return type of the cluster
 */
uint8_t Cluster::getClusterType() const
{
    return m_metaData.clusterType;
}


/**
 * @brief Cluster::initMessageBuffer initialize the message-buffer
 * @param controller pointer to the central message-controller
 */
void Cluster::initMessageBuffer(MessageController* controller)
{
    m_incomingMessageQueue = new IncomingMessageBuffer(this, controller);
    m_outgoingMessageQueue = new OutgoingMessageBuffer(this, controller);
}

/**
 * @brief Cluster::getIncomingMessageBuffer get the incoming message-buffer
 * @return pointer to the incoming message-buffer
 */
IncomingMessageBuffer* Cluster::getIncomingMessageBuffer()
{
    return m_incomingMessageQueue;
}

/**
 * @brief Cluster::getOutgoingMessageBuffer get the outgoing message-buffer
 * @return pointer to the outgoing message-buffer
 */
OutgoingMessageBuffer* Cluster::getOutgoingMessageBuffer()
{
    return m_outgoingMessageQueue;
}

/**
 * @brief Cluster::addNeighbor add a new neighbor to the cluster
 * @param side side for the new neighbor
 * @param target neighbor-object
 * @return true, if neighbor was added, else false
 */
bool Cluster::addNeighbor(const uint8_t side, const Neighbor target)
{
    // check if side is valid
    if(side > 17) {
        return false;
    }

    // add the new neighbor
    m_metaData.neighors[side] = target;
    m_metaData.neighors[side].targetSide = 16 - side;

    // update message-queue
    if(m_outgoingMessageQueue != nullptr) {
        m_outgoingMessageQueue->updateBufferInit();
    }
    // persist meta-data
    updateMetaData();
    return true;
}

/**
 * @brief Cluster::getNeighbors get the list with the neighbors of the cluster
 * @return pointer to the neighbor-list
 */
Neighbor* Cluster::getNeighbors()
{
    return &m_metaData.neighors[0];
}

/**
 * @brief Cluster::getNeighborId get the id of a special neighbor
 * @param side side of the requested neighbor
 * @return id of the neighbor
 */
ClusterID Cluster::getNeighborId(const uint8_t side)
{
    return m_metaData.neighors[side].targetClusterId;
}

/**
 * @brief Cluster::finishCycle finish the current cycle with sending messages from the outgoing buffer
 * @param numberOfActiveNodes number of active nodes in the current cluster
 */
void Cluster::finishCycle(const uint16_t numberOfActiveNodes)
{
    m_outgoingMessageQueue->finishCycle(0, numberOfActiveNodes);
    m_outgoingMessageQueue->finishCycle(2, numberOfActiveNodes);
    m_outgoingMessageQueue->finishCycle(3, numberOfActiveNodes);
    m_outgoingMessageQueue->finishCycle(4, numberOfActiveNodes);
    m_outgoingMessageQueue->finishCycle(8, numberOfActiveNodes);
    m_outgoingMessageQueue->finishCycle(12, numberOfActiveNodes);
    m_outgoingMessageQueue->finishCycle(13, numberOfActiveNodes);
    m_outgoingMessageQueue->finishCycle(14, numberOfActiveNodes);
    m_outgoingMessageQueue->finishCycle(16, numberOfActiveNodes);
}

}
