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
                 const std::string directoryPath)
{
    m_metaData.clusterId = clusterId;

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
    if(side == 0) {
        m_metaData.outgoing = 1;
    }
    if(side == 15) {
        m_metaData.incoming = 1;
    }
    m_metaData.neighors[side] = target;
    m_metaData.neighors[side].targetSide = 15 - side;
    if(m_outgoingMessageQueue != nullptr) {
        m_outgoingMessageQueue->updateBufferInit();
    }
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
    return m_metaData.neighors[side].targetClusterId;
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
    // TODO: readd file-path after tests
    m_clusterDataBuffer = new PerformanceIO::DataBuffer("");
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
IncomingMessageBuffer *Cluster::getIncomingMessageBuffer()
{
    return m_incomingMessageQueue;
}

/**
 * @brief Cluster::getOutgoingMessageBuffer
 * @return
 */
OutgoingMessageBuffer *Cluster::getOutgoingMessageBuffer()
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

/**
 * @brief Cluster::getNumberOfAxons
 * @return
 */
uint32_t Cluster::getNumberOfAxons() const
{
    return m_metaData.numberOfAxons;
}

/**
 * @brief Cluster::getNumberOfAxonBlocks
 * @return
 */
uint32_t Cluster::getNumberOfAxonBlocks() const
{
    return m_metaData.numberOfAxonBlocks;
}

/**
 * @brief Cluster::getAxonBlock
 * @return
 */
KyoChanAxon *Cluster::getAxonBlock()
{
    uint32_t positionAxonBlock = m_metaData.positionAxonBlocks;
    return (KyoChanAxon*)m_clusterDataBuffer->getBlock(positionAxonBlock);
}

/**
 * @brief Cluster::initAxonBlocks
 * @param numberOfAxons
 * @return
 */
bool Cluster::initAxonBlocks(const uint32_t numberOfAxons)
{
    if(m_metaData.numberOfAxons != 0 || numberOfAxons == 0) {
        return false;
    }

    m_metaData.numberOfAxons = numberOfAxons;
    m_metaData.positionAxonBlocks = m_metaData.positionNodeBlocks + m_metaData.numberOfNodeBlocks;

    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    m_metaData.numberOfAxonBlocks = (numberOfAxons * sizeof(KyoChanAxon)) / blockSize;
    if((numberOfAxons * sizeof(KyoChanAxon)) % blockSize != 0) {
        m_metaData.numberOfAxonBlocks += 1;
    }

    m_clusterDataBuffer->allocateBlocks(m_metaData.numberOfAxonBlocks);
    updateMetaData(m_metaData);

    initEdgeBlocks(numberOfAxons);

    // fill array with empty axons
    KyoChanAxon* array = getAxonBlock();
    for(uint32_t i = 0; i < numberOfAxons; i++)
    {
        KyoChanAxon tempAxon;
        tempAxon.edgeSectionId = i;
        array[i] = tempAxon;
    }
    m_clusterDataBuffer->syncAll();
    return true;
}

/**
 * @brief Cluster::finishCycle
 */
void Cluster::finishCycle()
{
    OUTPUT("---")
    OUTPUT("finishCycle")
    m_outgoingMessageQueue->finishCycle(2);
    m_outgoingMessageQueue->finishCycle(3);
    m_outgoingMessageQueue->finishCycle(4);
    m_outgoingMessageQueue->finishCycle(11);
    m_outgoingMessageQueue->finishCycle(12);
    m_outgoingMessageQueue->finishCycle(13);
}


/**
 * @brief Cluster::syncEdgeSections
 * @param startSection
 * @param endSection
 */
void Cluster::syncEdgeSections(uint32_t startSection,
                               uint32_t endSection)
{
    if(endSection < startSection) {
        startSection = 0;
    }
    startSection += m_metaData.positionOfEdgeBlock;
    endSection += m_metaData.positionOfEdgeBlock;
    m_clusterDataBuffer->syncBlocks(startSection, endSection);
}

}
