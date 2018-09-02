/**
 *  @file    nodeCluster.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/clustering/cluster/nodeCluster.h>
#include <files/dataBuffer.h>
#include <messageQueues/incomingMessageBuffer.h>
#include <messageQueues/outgoingMessageBuffer.h>
#include <kyochanNetwork.h>

namespace KyoukoMind
{

/**
 * @brief NodeCluster::NodeCluster constructor for node-cluster-creation
 * @param clusterId id for the new cluster
 * @param directoryPath path to the directory, where the file of the cluster should be saved
 * @param numberOfNodes number of nodes of the new node-cluster
 */
NodeCluster::NodeCluster(const ClusterID clusterId,
                         const std::string directoryPath,
                         const uint32_t numberOfNodes)
    : Cluster(clusterId,
              directoryPath)
{
    m_metaData.clusterType = NODE_CLUSTER;
    m_metaData.positionNodeBlocks = 1;
    initNodeBlocks(numberOfNodes);
}

/**
 * @brief NodeCluster::getNumberOfNodeBlocks get number of node-blocks of the cluster in the buffer
 * @return number of node-blocks
 */
uint32_t NodeCluster::getNumberOfNodeBlocks() const
{
    return m_metaData.numberOfNodeBlocks;
}

/**
 * @brief NodeCluster::getNumberOfNode get number of nodes of the cluster
 * @return number of nodes
 */
uint32_t NodeCluster::getNumberOfNodes() const
{
    return m_metaData.numberOfNodes;
}

/**
 * @brief NodeCluster::getNodeBlock get pointer to the node-list of the cluster
 * @return pointer to the first block of the node-list
 */
KyoChanNode *NodeCluster::getNodeBlock()
{
    uint32_t positionNodeBlock = m_metaData.positionNodeBlocks;
    return (KyoChanNode*)m_clusterDataBuffer->getBlock(positionNodeBlock);
}

/**
 * @brief NodeCluster::initNodeBlocks initialize the node-list of the cluster
 * @param numberOfNodes number of new empty nodes
 * @return false if nodes are already initialized, esle true
 */
bool NodeCluster::initNodeBlocks(uint32_t numberOfNodes)
{
    // prechecks
    if(m_metaData.numberOfNodes != 0) {
        return false;
    }

    if(numberOfNodes == 0) {
        numberOfNodes = NUMBER_OF_NODES_PER_CLUSTER;
    }

    // update meta-data of the cluster
    m_metaData.numberOfNodes = numberOfNodes;
    m_metaData.positionNodeBlocks = 1;

    const uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    m_metaData.numberOfNodeBlocks = (numberOfNodes * sizeof(KyoChanNode)) / blockSize + 1;

    // allocate blocks in buffer
    m_clusterDataBuffer->allocateBlocks(m_metaData.numberOfNodeBlocks);
    updateMetaData();

    // fill array with empty nodes
    KyoChanNode* array = getNodeBlock();
    for(uint16_t i = 0; i < numberOfNodes; i++)
    {
        KyoChanNode tempNode;
        array[i] = tempNode;
    }

    m_metaData.positionOfEdgeBlock = m_metaData.positionNodeBlocks + m_metaData.numberOfNodeBlocks;

    // write the new init nodes to the buffer and the file
    m_clusterDataBuffer->syncAll();
    return true;
}

/**
 * @brief NodeCluster::getNumberOfEdgeSectionBlocks
 * @return
 */
uint32_t NodeCluster::getNumberOfEdgeSectionBlocks() const
{
    return m_metaData.numberOfEdgeBlocks;
}

/**
 * @brief NodeCluster::getNumberOfEdgeSections
 * @return
 */
uint32_t NodeCluster::getNumberOfEdgeSections() const
{
    return m_metaData.numberOfEdgeSections;
}

/**
 * @brief NodeCluster::getEdgeSectionBlock
 * @return
 */
KyoChanEdgeSection *NodeCluster::getEdgeSectionBlock()
{
    uint32_t positionEdgeBlock = m_metaData.positionOfEdgeBlock;
    return (KyoChanEdgeSection*)m_clusterDataBuffer->getBlock(positionEdgeBlock);
}

/**
 * @brief NodeCluster::initEdgeSectionBlocks
 * @param numberOfEdgeSections
 * @return
 */
bool NodeCluster::initEdgeSectionBlocks(const uint32_t numberOfEdgeSections)
{
    // prechecks
    if(m_metaData.numberOfEdgeSections != 0 || numberOfEdgeSections == 0) {
        return false;
    }

    // update meta-data of the cluster
    m_metaData.numberOfEdgeSections = numberOfEdgeSections;
    m_metaData.positionOfEdgeBlock = m_metaData.numberOfNodeBlocks + 1;

    // calculate number of edge-blocks
    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    m_metaData.numberOfEdgeBlocks = (numberOfEdgeSections * sizeof(KyoChanEdgeSection)) / blockSize;
    if((numberOfEdgeSections * sizeof(KyoChanEdgeSection)) % blockSize != 0) {
        m_metaData.numberOfEdgeBlocks += 1;
    }

    // update and persist buffer
    m_clusterDataBuffer->allocateBlocks(m_metaData.numberOfEdgeBlocks);
    updateMetaData();

    // fill array with empty edgesections
    KyoChanEdgeSection* array = getEdgeSectionBlock();
    for(uint32_t i = 0; i < numberOfEdgeSections; i++)
    {
        KyoChanEdgeSection newSection;
        newSection.sourceId = i;
        array[i] = newSection;
    }
    // write the new init nodes to the buffer and the file
    m_clusterDataBuffer->syncAll();
    return true;
}

/**
 * @brief NodeCluster::addEdge add an existing edge to a specifig edge-sections
 * @param edgeSectionId id of the edge-section for the new edge
 * @param newEdge new edge, which should be added
 * @return false, if edgeSectionId is too big, else true
 */
bool NodeCluster::addEdge(const uint32_t edgeSectionId, const KyoChanEdge &newEdge)
{
    // check if id is valid
    if(edgeSectionId >= m_metaData.numberOfEdgeSections) {
        return false;
    }

    // get section and add the new edge
    KyoChanEdgeSection* edgeSection = &getEdgeSectionBlock()[edgeSectionId];
    return edgeSection->addEdge(newEdge);
}

/**
 * @brief NodeCluster::addEmptyEdgeSection add a new empfy edge-section
 * @return id of the new section, or SPECIAL_STATE if memory-allocation failed
 * @param sourceId id of the source forward-edge-section
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
uint32_t NodeCluster::addEmptyEdgeSection(const uint8_t sourceSide,
                                          const uint32_t sourceId)
{
    // allocate a new block, if necessary
    const uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    if((m_metaData.numberOfEdgeSections * sizeof(KyoChanEdgeSection) )/ blockSize
            < ((m_metaData.numberOfEdgeSections + 1) * sizeof(KyoChanEdgeSection)) / blockSize)
    {
        if(!m_clusterDataBuffer->allocateBlocks(1)) {
            return UNINIT_STATE_32;
        }
        m_metaData.numberOfEdgeBlocks++;
    }

    // add new edge-forward-section
    KyoChanEdgeSection newSection;
    newSection.sourceId = sourceId;
    newSection.sourceSide = sourceSide;
    getEdgeSectionBlock()[m_metaData.numberOfEdgeSections] = newSection;

    m_metaData.numberOfEdgeSections++;
    return m_metaData.numberOfEdgeSections-1;
}

}
