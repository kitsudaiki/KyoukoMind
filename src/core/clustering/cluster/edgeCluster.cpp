/**
 *  @file    edgeCluster.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/clustering/cluster/edgeCluster.h>
#include <files/dataBuffer.h>
#include <core/messaging/messageController.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

namespace KyoukoMind
{

/**
 * @brief EdgeCluster::EdgeCluster constructor for edge-cluster-creation
 * @param clusterId id for the new cluster
 * @param directoryPath path to the directory, where the file of the cluster should be saved
 */
EdgeCluster::EdgeCluster(const ClusterID &clusterId,
                         const std::string directoryPath)
{
    // set meta-data
    m_metaData.clusterType = EDGE_CLUSTER;
    m_metaData.clusterId = clusterId;

    // creates a file for the cluster with buffer
    // TODO: readd file-path after tests
    std::string filePath = directoryPath
                         + "/cluster_" + std::to_string(clusterId);
    m_clusterDataBuffer = new PerformanceIO::DataBuffer("");

    // init the buffer and the file with the first block for the meta-data
    m_clusterDataBuffer->allocateBlocks(1);
    updateMetaData();
}

/**
 * @brief EdgeCluster::~EdgeCluster destroy the buffer while destroying the cluster-object
 */
EdgeCluster::~EdgeCluster()
{
    if(m_clusterDataBuffer != nullptr)
    {
        m_clusterDataBuffer->closeBuffer();
        delete m_clusterDataBuffer;
        m_clusterDataBuffer = nullptr;
    }
}

/**
 * @brief EdgeCluster::getMetaData reads the meta-data from the file-buffer
 */
void EdgeCluster::getMetaData()
{
    ClusterMetaData* metaDataPointer = (ClusterMetaData*)m_clusterDataBuffer->getBlock(0);
    m_metaData = *metaDataPointer;
}


/**
 * @brief EdgeCluster::updateMetaData write the current mata-data to the buffer and the file
 */
void EdgeCluster::updateMetaData()
{
    uint32_t size = sizeof(ClusterMetaData);
    void* metaDataBlock = m_clusterDataBuffer->getBlock(0);
    memcpy(metaDataBlock, &m_metaData, size);
    m_clusterDataBuffer->syncBlocks(0, 0);
}

/**
 * @brief EdgeCluster::getClusterId returns the cluster-id from the meta-data
 * @return id of the cluster
 */
ClusterID EdgeCluster::getClusterId() const
{
    return m_metaData.clusterId;
}

/**
 * @brief EdgeCluster::getClusterType returns the cluster-type from the meta-data
 * @return type of the cluster
 */
uint8_t EdgeCluster::getClusterType() const
{
    return m_metaData.clusterType;
}


/**
 * @brief EdgeCluster::initMessageBuffer initialize the message-buffer
 * @param controller pointer to the central message-controller
 */
void EdgeCluster::initMessageBuffer(MessageController* controller)
{
    m_incomingMessageQueue = new IncomingMessageBuffer(this, controller);
    m_outgoingMessageQueue = new OutgoingMessageBuffer(this, controller);
}

/**
 * @brief EdgeCluster::getIncomingMessageBuffer get the incoming message-buffer
 * @return pointer to the incoming message-buffer
 */
IncomingMessageBuffer* EdgeCluster::getIncomingMessageBuffer()
{
    return m_incomingMessageQueue;
}

/**
 * @brief EdgeCluster::getOutgoingMessageBuffer get the outgoing message-buffer
 * @return pointer to the outgoing message-buffer
 */
OutgoingMessageBuffer* EdgeCluster::getOutgoingMessageBuffer()
{
    return m_outgoingMessageQueue;
}

/**
 * @brief EdgeCluster::addNeighbor add a new neighbor to the cluster
 * @param side side for the new neighbor
 * @param target neighbor-object
 * @return true, if neighbor was added, else false
 */
bool EdgeCluster::addNeighbor(const uint8_t side, const Neighbor target)
{
    // check if side is valid
    if(side > 16) {
        return false;
    }

    // add spezial state, if an input- or output-neighbor will be added
    if(side == 0) {
        m_metaData.incoming = 1;
    }
    if(side == 15) {
        m_metaData.outgoing = 1;
    }

    // add the new neighbor
    m_metaData.neighors[side] = target;
    m_metaData.neighors[side].targetSide = 15 - side;

    // update message-queue
    if(m_outgoingMessageQueue != nullptr) {
        m_outgoingMessageQueue->updateBufferInit();
    }
    // persist meta-data
    updateMetaData();
    return true;
}

/**
 * @brief EdgeCluster::getNeighbors get the list with the neighbors of the cluster
 * @return pointer to the neighbor-list
 */
Neighbor* EdgeCluster::getNeighbors()
{
    return &m_metaData.neighors[0];
}

/**
 * @brief EdgeCluster::getNeighborId get the id of a special neighbor
 * @param side side of the requested neighbor
 * @return id of the neighbor
 */
ClusterID EdgeCluster::getNeighborId(const uint8_t side)
{
    return m_metaData.neighors[side].targetClusterId;
}

/**
 * @brief EdgeCluster::getNumberOfForwardEdgeSectionBlocks get the number of forward-edge-section-block from meta-data
 * @return number of forward-edge-section-blocks
 */
uint32_t EdgeCluster::getNumberOfForwardEdgeSectionBlocks() const
{
    return m_metaData.numberOfForwardEdgeBlocks;
}

/**
 * @brief EdgeCluster::getForwardEdgeSectionBlock get the forward-edge-section-block
 * @return pointer to the beginning of the forward-edge-section-block
 */
KyoChanForwardEdgeSection *EdgeCluster::getForwardEdgeSectionBlock()
{
    uint32_t positionForwardEdgeBlocks = m_metaData.positionForwardEdgeBlocks;
    return (KyoChanForwardEdgeSection*)m_clusterDataBuffer->getBlock(positionForwardEdgeBlocks);
}

/**
 * @brief EdgeCluster::allocForwardEdgeSectionBlocks allocates a number of new forward-edge-sections
 * @param numberOfForwardEdgeSections number of new forward-edge-sections
 * @return 0xFFFFFFFF if failed, else number of the last allocated edge-sections
 */
uint32_t EdgeCluster::allocForwardEdgeSectionBlocks(const uint32_t numberOfForwardEdgeSections)
{
    if(numberOfForwardEdgeSections == 0) {
        return 0xFFFFFFFF;
    }

    // calculate number of edge-blocks
    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    uint32_t newSectionNumber = m_metaData.numberOfForwardEdgeSections + numberOfForwardEdgeSections;
    uint32_t newBlockNumber = (newSectionNumber * sizeof(KyoChanForwardEdgeSection)) / blockSize;
    if((newSectionNumber * sizeof(KyoChanForwardEdgeSection)) % blockSize != 0) {
        newBlockNumber += 1;
    }

    // update and persist buffer
    if(!m_clusterDataBuffer->allocateBlocks(newBlockNumber - m_metaData.numberOfEdgeBlocks)) {
        return 0xFFFFFFFF;
    }
    updateMetaData();

    // fill array with empty edgesections
    KyoChanForwardEdgeSection* array = getForwardEdgeSectionBlock();
    for(uint32_t i = m_metaData.numberOfForwardEdgeSections;
        i < numberOfForwardEdgeSections;
        i++)
    {
        KyoChanForwardEdgeSection newSection;
        array[i] = newSection;
    }

    // update meta-dat
    m_metaData.numberOfForwardEdgeSections = newSectionNumber;
    m_metaData.numberOfForwardEdgeBlocks = newBlockNumber;
    m_metaData.positionForwardEdgeBlocks = m_metaData.positionNodeBlocks + m_metaData.numberOfNodeBlocks;

    // persist changes
    updateMetaData();
    m_clusterDataBuffer->syncAll();

    return m_metaData.numberOfForwardEdgeSections-1;
}

/**
 * @brief Cluster::finishCycle finish the current cycle with sending messages from the outgoing buffer
 */
void EdgeCluster::finishCycle()
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

}
