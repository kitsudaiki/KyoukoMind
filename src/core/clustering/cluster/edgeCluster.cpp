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
 * @brief Cluster::getNumberOfAxons
 * @return
 */
uint32_t EdgeCluster::getNumberOfAxons() const
{
    return m_metaData.numberOfAxons;
}

/**
 * @brief Cluster::getNumberOfAxonBlocks
 * @return
 */
uint32_t EdgeCluster::getNumberOfAxonBlocks() const
{
    return m_metaData.numberOfAxonBlocks;
}

/**
 * @brief Cluster::getAxonBlock
 * @return
 */
KyoChanAxon *EdgeCluster::getAxonBlock()
{
    uint32_t positionAxonBlock = m_metaData.positionAxonBlocks;
    return (KyoChanAxon*)m_clusterDataBuffer->getBlock(positionAxonBlock);
}

/**
 * @brief Cluster::initAxonBlocks
 * @param numberOfAxons
 * @return
 */
bool EdgeCluster::initAxonBlocks(const uint32_t numberOfAxons)
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
    updateMetaData();

    initForwardEdgeSectionBlocks(numberOfAxons);

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

/**
 * @brief Cluster::syncEdgeSections
 * @param startSection
 * @param endSection
 */
void EdgeCluster::syncEdgeSections(uint32_t startSection,
                               uint32_t endSection)
{
    if(endSection < startSection) {
        startSection = 0;
    }
    startSection += m_metaData.positionOfEdgeBlock;
    endSection += m_metaData.positionOfEdgeBlock;
    m_clusterDataBuffer->syncBlocks(startSection, endSection);
}


/**
 * @brief EdgeCluster::getNumberOfEdgeBlocks
 * @return
 */
uint32_t EdgeCluster::getNumberOfForwardEdgeSectionBlocks() const
{
    return m_metaData.numberOfEdgeBlocks;
}

/**
 * @brief EdgeCluster::getEdgeBlock
 * @return
 */
KyoChanForwardEdgeSection *EdgeCluster::getForwardEdgeSectionBlock()
{
    uint32_t positionEdgeBlock = m_metaData.positionOfEdgeBlock;
    return (KyoChanForwardEdgeSection*)m_clusterDataBuffer->getBlock(positionEdgeBlock);
}

/**
 * @brief EdgeCluster::initEdgeBlocks
 * @param numberOfEdgeSections
 * @return
 */
bool EdgeCluster::initForwardEdgeSectionBlocks(const uint32_t numberOfForwardEdgeSections)
{
    if(m_metaData.numberOfEdgeSections != 0 || numberOfForwardEdgeSections == 0) {
        return false;
    }

    m_metaData.numberOfEdgeSections = numberOfForwardEdgeSections;
    m_metaData.positionOfEdgeBlock = m_metaData.positionAxonBlocks + m_metaData.numberOfAxonBlocks;

    // calculate number of edge-blocks
    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    m_metaData.numberOfEdgeBlocks = (numberOfForwardEdgeSections * sizeof(KyoChanForwardEdgeSection)) / blockSize;
    if((numberOfForwardEdgeSections * sizeof(KyoChanForwardEdgeSection)) % blockSize != 0) {
        m_metaData.numberOfEdgeBlocks += 1;
    }

    // update and persist buffer
    m_clusterDataBuffer->allocateBlocks(m_metaData.numberOfEdgeBlocks);
    updateMetaData();

    // fill array with empty edgesections
    KyoChanForwardEdgeSection* array = getForwardEdgeSectionBlock();
    for(uint32_t i = 0; i < numberOfForwardEdgeSections; i++)
    {
        KyoChanForwardEdgeSection newSection;
        array[i] = newSection;
    }
    m_clusterDataBuffer->syncAll();
    return true;
}

/**
 * @brief EdgeCluster::addEmptyEdgeForwardSection
 * @return
 */
uint32_t EdgeCluster::addEmptyForwardEdgeSection()
{
    // allocate a new block, if necessary
    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    if((m_metaData.numberOfEdgeSections * sizeof(KyoChanForwardEdgeSection) )/ blockSize
            < ((m_metaData.numberOfEdgeSections + 1) * sizeof(KyoChanForwardEdgeSection)) / blockSize)
    {
        // TODO: outsourcing
        if(!m_clusterDataBuffer->allocateBlocks(1)) {
            return 0xFFFFFFFF;
        }
        m_metaData.numberOfEdgeBlocks++;
        m_metaData.numberOfPendingEdgeSections++;
    }

    // add new edge-forward-section
    KyoChanForwardEdgeSection newSection;
    getForwardEdgeSectionBlock()[m_metaData.numberOfEdgeSections] = newSection;
    m_metaData.numberOfEdgeSections++;
    return m_metaData.numberOfEdgeSections-1;
}

}
