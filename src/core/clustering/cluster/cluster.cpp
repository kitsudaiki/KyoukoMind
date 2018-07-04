#include "cluster.h"

#include <files/dataBuffer.h>
#include <messageQueues/incomingMessageBuffer.h>
#include <messageQueues/outgoingMessageBuffer.h>
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
    m_metaData.clusterId = clusterId;

    // creates a file for the cluster with buffer
    // TODO: readd file-path after tests
    std::string filePath = directoryPath
                         + "/cluster_" + std::to_string(clusterId);
    m_clusterDataBuffer = new PerformanceIO::DataBuffer("");

    // init the buffer and the file with the first block for the meta-data
    m_clusterDataBuffer->allocateBlocks(1);
    updateMetaData();

    // init empty neighbor-list to avoid not initialized values
    for(uint32_t i = 0; i < 17; i++)
    {
        Neighbor emptyNeighbor;
        emptyNeighbor.targetClusterId = UNINIT_STATE;
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
    if(this != nullptr) {
        return m_metaData;
    }
    ClusterMetaData tempMeta;
    tempMeta.clusterId = UNINIT_STATE;
    return tempMeta;
}

/**
 * @brief Cluster::getMetaDataFromBuffer reads the meta-data from the file-buffer
 */
void Cluster::getMetaDataFromBuffer()
{
    if(this == nullptr) {
        return;
    }
    ClusterMetaData* metaDataPointer = (ClusterMetaData*)m_clusterDataBuffer->getBlock(0);
    m_metaData = *metaDataPointer;
}


/**
 * @brief Cluster::updateMetaData write the current mata-data to the buffer and the file
 */
void Cluster::updateMetaData()
{
    if(this == nullptr) {
        return;
    }
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
    if(this != nullptr) {
        return m_metaData.clusterId;
    }
    return UNINIT_STATE;
}

/**
 * @brief Cluster::getClusterType returns the cluster-type from the meta-data
 * @return type of the cluster
 */
uint8_t Cluster::getClusterType() const
{
    if(this != nullptr) {
        return m_metaData.clusterType;
    }
    return UNINIT_STATE;
}

/**
 * @brief Cluster::isExising check if the instance of the Cluster-class exist
 * @return true, if exist, else false
 */
bool Cluster::isExising() const
{
    if(this != nullptr) {
        return true;
    }
    return false;
}

/**
 * @brief Cluster::getIncomingMessageBuffer get the incoming message-buffer
 * @return pointer to the incoming message-buffer
 */
Networking::IncomingMessageBuffer* Cluster::getIncomingMessageBuffer(const uint8_t side)
{
    // TODO: method-comment
    if(this != nullptr && side < 17) {
        return m_metaData.neighors[side].incomBuffer;
    }
    return nullptr;
}

/**
 * @brief Cluster::getOutgoingMessageBuffer get the outgoing message-buffer
 * @return pointer to the outgoing message-buffer
 */
Networking::OutgoingMessageBuffer* Cluster::getOutgoingMessageBuffer(const uint8_t side)
{
    // TODO: method-comment
    if(this != nullptr && side < 17) {
        return m_metaData.neighors[side].outgoBuffer;
    }
    return nullptr;
}

/**
 * @brief Cluster::setIncomingMessageBuffer
 * @param side
 * @param buffer
 * @return
 */
bool Cluster::setNewConnection(const uint8_t side,
                               Networking::IncomingMessageBuffer *buffer)
{
    // TODO: method-comment
    if(this != nullptr && side < 17)
    {
        m_metaData.neighors[side].outgoBuffer->setIncomingBuffer(buffer);
        return true;
    }
    return false;
}

/**
 * @brief Cluster::setNeighbor set a new neighbor to the cluster
 * @param side side for the new neighbor
 * @param targetClusterId id of the neighbor-target
 * @return true, if neighbor was added, else false
 */
bool Cluster::setNeighbor(const uint8_t side, const ClusterID targetClusterId)
{
    // check if side is valid
    if(this != nullptr && side < 17)
    {
        // add the new neighbor
        m_metaData.neighors[side].targetClusterId = targetClusterId;
        m_metaData.neighors[side].targetSide = 16 - side;
        m_metaData.neighors[side].incomBuffer = new Networking::IncomingMessageBuffer();
        m_metaData.neighors[side].outgoBuffer = new Networking::OutgoingMessageBuffer();

        // persist meta-data
        updateMetaData();
        return true;
    }
    return false;
}

/**
 * @brief Cluster::getNeighbors get the list with the neighbors of the cluster
 * @return pointer to the neighbor-list
 */
Neighbor* Cluster::getNeighbors()
{
    if(this != nullptr) {
        return &m_metaData.neighors[0];
    }
    return nullptr;
}

/**
 * @brief Cluster::getNeighborId get the id of a special neighbor
 * @param side side of the requested neighbor
 * @return id of the neighbor
 */
ClusterID Cluster::getNeighborId(const uint8_t side)
{
    if(this != nullptr && side < 17) {
        return m_metaData.neighors[side].targetClusterId;
    }
    return UNINIT_STATE;
}

/**
 * @brief Cluster::finishCycle finish the current cycle with sending messages from the outgoing buffer
 * @param numberOfActiveNodes number of active nodes in the current cluster
 */
void Cluster::finishCycle(const uint16_t numberOfActiveNodes)
{
    if(this == nullptr) {
        return;
    }
    for(uint8_t side = 0; side < 17; side++)
    {
        if(m_metaData.neighors[side].targetClusterId != UNINIT_STATE)
        {
            m_metaData.neighors[side].outgoBuffer->finishCycle(m_metaData.neighors[side].targetClusterId,
                                                               16 - side,
                                                               m_metaData.clusterId,
                                                               numberOfActiveNodes);
        }
    }
}

}
