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
#include <kyochanNetwork.h>

namespace KyoukoMind
{

/**
 * @brief EdgeCluster::EdgeCluster constructor for edge-cluster-creation
 * @param clusterId id for the new cluster
 * @param directoryPath path to the directory, where the file of the cluster should be saved
 */
EdgeCluster::EdgeCluster(const ClusterID &clusterId,
                         const std::string directoryPath) :
    Cluster(clusterId, directoryPath)
{
    // set meta-data
    m_metaData.clusterType = EDGE_CLUSTER;
    m_metaData.clusterId = clusterId;
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

}
