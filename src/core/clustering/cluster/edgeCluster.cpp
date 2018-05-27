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

    // init empty neighbor-list to avoid not initialized values
    for(uint32_t i = 0; i < 17; i++) {
        Neighbor emptyNeighbor;
        emptyNeighbor.targetSide = 16 - i;
        m_metaData.neighors[i] = emptyNeighbor;
    }
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
 * @brief EdgeCluster::getMetaData get the meta-data of the cluster
 * @return meta-data-object
 */
ClusterMetaData EdgeCluster::getMetaData() const
{
    return m_metaData;
}

/**
 * @brief EdgeCluster::getMetaDataFromBuffer reads the meta-data from the file-buffer
 */
void EdgeCluster::getMetaDataFromBuffer()
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
    if(side > 17) {
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
 * @brief EdgeCluster::initForwardEdgeSectionBlocks initialize forward-edge-block
 * @param numberOfForwardEdgeSections number of forward-edge-sections
 * @return true if success, else false
 */
bool EdgeCluster::initForwardEdgeSectionBlocks(const uint32_t numberOfForwardEdgeSections)
{
    // prechecks
    if(m_metaData.numberOfEdgeSections != 0 || numberOfForwardEdgeSections == 0) {
        return false;
    }

    // update meta-data of the cluster
    m_metaData.numberOfForwardEdgeSections = numberOfForwardEdgeSections;
    m_metaData.positionForwardEdgeBlocks = 1;

    // calculate number of edge-blocks
    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    m_metaData.numberOfForwardEdgeBlocks= (numberOfForwardEdgeSections * sizeof(KyoChanForwardEdgeSection)) / blockSize;
    if((numberOfForwardEdgeSections * sizeof(KyoChanForwardEdgeSection)) % blockSize != 0) {
        m_metaData.numberOfForwardEdgeBlocks += 1;
    }

    // update and persist buffer
    m_clusterDataBuffer->allocateBlocks(m_metaData.numberOfForwardEdgeBlocks);
    updateMetaData();

    // fill array with empty forward-edge-sections
    KyoChanForwardEdgeSection* array = getForwardEdgeSectionBlock();
    for(uint32_t i = 0; i < numberOfForwardEdgeSections; i++)
    {
        KyoChanForwardEdgeSection newSection;
        array[i] = newSection;
    }
    // write the new init nodes to the buffer and the file
    m_clusterDataBuffer->syncAll();
    return true;
}

/**
 * @brief EdgeCluster::addEmptyForwardEdgeSection add a new forward-edge-section
 * @param sourceSide id of the incoming side
 * @param sourceId id of the source forward-edge-section
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
uint32_t EdgeCluster::addEmptyForwardEdgeSection(const uint8_t sourceSide,
                                                 const uint32_t sourceId)
{
    // allocate a new block, if necessary
    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    if((m_metaData.numberOfForwardEdgeSections * sizeof(KyoChanForwardEdgeSection) )/ blockSize
            < ((m_metaData.numberOfForwardEdgeSections + 1) * sizeof(KyoChanForwardEdgeSection)) / blockSize)
    {
        if(!m_clusterDataBuffer->allocateBlocks(1)) {
            return UNINIT_STATE;
        }
        m_metaData.numberOfForwardEdgeBlocks++;
    }

    // add new edge-forward-section
    KyoChanForwardEdgeSection newSection;
    newSection.sourceId = sourceId;
    newSection.sourceSide = sourceSide;
    getForwardEdgeSectionBlock()[m_metaData.numberOfForwardEdgeSections] = newSection;
    m_metaData.numberOfForwardEdgeSections++;
    m_metaData.numberOfPendingForwardEdgeSections++;
    return m_metaData.numberOfForwardEdgeSections-1;
}

/**
 * @brief EdgeCluster::getPendingForwardEdgeSectionBlock get pending-forward-edges
 * @return nullptr if no pending edge exist, else pointer to the beginning of the pending edges
 */
KyoChanForwardEdgeSection *EdgeCluster::getPendingForwardEdgeSectionBlock()
{
    if(m_metaData.numberOfPendingForwardEdgeSections == 0)
    {
        return nullptr;
    }
    return &(getForwardEdgeSectionBlock()[m_metaData.numberOfForwardEdgeSections
            - m_metaData.numberOfPendingForwardEdgeSections]);
}

/**
 * @brief EdgeCluster::getPendingForwardEdgeSectionId
 * @return
 */
uint32_t EdgeCluster::getPendingForwardEdgeSectionId() const
{
    if(m_metaData.numberOfPendingForwardEdgeSections == 0) {
        return UNINIT_STATE;
    }
    return m_metaData.numberOfForwardEdgeSections - m_metaData.numberOfPendingForwardEdgeSections;
}

/**
 * @brief EdgeCluster::decreaseNumberOfPendingForwardEdges reduces the number of pending edges by one
 */
void EdgeCluster::decreaseNumberOfPendingForwardEdges()
{
    m_metaData.numberOfPendingForwardEdgeSections--;
}

/**
 * @brief EdgeCluster::finishCycle finish the current cycle with sending messages from the outgoing buffer
 * @param numberOfActiveNodes number of active nodes in the current cluster
 */
void EdgeCluster::finishCycle(const uint16_t numberOfActiveNodes)
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
