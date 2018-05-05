/**
 *  @file    nodeCluster.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/clustering/cluster/nodeCluster.h>
#include <kyochanNetwork.h>
#include <files/dataBuffer.h>

namespace KyoukoMind
{

/**
 * @brief NodeCluster::NodeCluster
 * @param clusterId
 * @param directoryPath
 * @param numberOfNodes
 */
NodeCluster::NodeCluster(const ClusterID clusterId,
                         const std::string directoryPath,
                         const uint32_t numberOfNodes)
    : Cluster(clusterId,
              NODE_CLUSTER,
              directoryPath)
{
    initNodeBlocks(numberOfNodes);
}

/**
 * @brief NodeCluster::~NodeCluster
 */
NodeCluster::~NodeCluster()
{}

/**
 * @brief NodeCluster::getNumberOfNode
 * @return
 */
uint32_t NodeCluster::getNumberOfNodeBlocks() const
{
    return m_metaData.numberOfNodeBlocks;
}

/**
 * @brief NodeCluster::getNumberOfNode
 * @return
 */
uint16_t NodeCluster::getNumberOfNodes() const
{
    return m_metaData.numberOfNodes;
}

/**
 * @brief NodeCluster::getNodeBlock
 * @return
 */
KyoChanNode *NodeCluster::getNodeBlock()
{
    uint32_t positionNodeBlock = m_metaData.positionNodeBlocks;
    return (KyoChanNode*)m_clusterDataBuffer->getBlock(positionNodeBlock);
}

/**
 * @brief NodeCluster::getEdgeBlock
 * @return
 */
KyoChanEdgeSection *NodeCluster::getEdgeBlock()
{
    uint32_t positionEdgeBlock = m_metaData.positionOfEdgeBlock;
    return (KyoChanEdgeSection*)m_clusterDataBuffer->getBlock(positionEdgeBlock);
}

/**
 * @brief NodeCluster::addOutputCluster
 * @param clusterId
 * @param outputAxonId
 * @param nodeId
 */
void NodeCluster::addOutputCluster(const ClusterID clusterId,
                                   const uint32_t outputAxonId,
                                   const uint16_t nodeId)
{
    m_metaData.outgoing = 1;
    Neighbor neighbor;
    neighbor.neighborType = EDGE_CLUSTER;
    neighbor.targetClusterId = clusterId;
    addNeighbor(15, neighbor);
    getNodeBlock()[nodeId].targetAxonId = outputAxonId;
    getNodeBlock()[nodeId].targetClusterPath = 15;
}

/**
 * @brief NodeCluster::initNodeBlocks
 * @param numberOfNodes
 * @return
 */
bool NodeCluster::initNodeBlocks(uint16_t numberOfNodes)
{
    if(numberOfNodes == 0 || m_metaData.numberOfNodes != 0) {
        return false;
    }

    m_metaData.numberOfNodes = numberOfNodes;
    m_metaData.positionNodeBlocks = 1;

    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    m_metaData.numberOfNodeBlocks = (numberOfNodes * sizeof(KyoChanNode)) / blockSize + 1;

    m_clusterDataBuffer->allocateBlocks(m_metaData.numberOfNodeBlocks);
    updateMetaData(m_metaData);

    // fill array with empty nodes
    KyoChanNode* array = getNodeBlock();
    for(uint16_t i = 0; i < numberOfNodes; i++)
    {
        KyoChanNode tempNode;
        array[i] = tempNode;
    }
    m_clusterDataBuffer->syncAll();
    return true;
}

/**
 * @brief NodeCluster::initEdgeBlocks
 * @param numberOfEdgeSections
 * @return
 */
bool NodeCluster::initEdgeBlocks(const uint32_t numberOfEdgeSections)
{
    if(m_metaData.numberOfEdgeSections != 0 || numberOfEdgeSections == 0) {
        return false;
    }

    m_metaData.numberOfEdgeSections = numberOfEdgeSections;
    m_metaData.positionOfEdgeBlock = m_metaData.positionAxonBlocks + m_metaData.numberOfAxonBlocks;

    // calculate number of edge-blocks
    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    m_metaData.numberOfEdgeBlocks = (numberOfEdgeSections * sizeof(KyoChanEdgeSection)) / blockSize;
    if((numberOfEdgeSections * sizeof(KyoChanEdgeSection)) % blockSize != 0) {
        m_metaData.numberOfEdgeBlocks += 1;
    }

    // update and persist buffer
    m_clusterDataBuffer->allocateBlocks(m_metaData.numberOfEdgeBlocks);
    updateMetaData(m_metaData);

    // fill array with empty edgesections
    KyoChanEdgeSection* array = getEdgeBlock();
    for(uint32_t i = 0; i < numberOfEdgeSections; i++)
    {
        KyoChanEdgeSection newSection;
        array[i] = newSection;
    }
    m_clusterDataBuffer->syncAll();
    return true;
}

/**
 * @brief NodeCluster::addEdge
 * @param edgeSectionId
 * @param newEdge
 * @return
 */
bool NodeCluster::addEdge(const uint32_t edgeSectionId, const KyoChanEdge &newEdge)
{
    if(edgeSectionId >= m_metaData.numberOfEdgeSections) {
        return false;
    }

    // get values
    KyoChanEdgeSection* edgeSection = &getEdgeBlock()[edgeSectionId];
    edgeSection->addEdge(newEdge);

    return true;
}

/**
 * @brief NodeCluster::addEmptyEdgeSection
 * @return
 */
uint32_t NodeCluster::addEmptyEdgeSection()
{
    // allocate a new block, if necessary
    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    if((m_metaData.numberOfEdgeSections * sizeof(KyoChanEdgeSection) )/ blockSize
            < ((m_metaData.numberOfEdgeSections + 1) * sizeof(KyoChanEdgeSection)) / blockSize)
    {
        // TODO: outsourcing
        if(!m_clusterDataBuffer->allocateBlocks(1)) {
            return 0xFFFFFFFF;
        }
        m_metaData.numberOfEdgeBlocks++;
        m_metaData.numberOfPendingEdgeSections++;
    }

    // add new edge-forward-section
    KyoChanEdgeSection newSection;
    getEdgeBlock()[m_metaData.numberOfEdgeSections] = newSection;
    m_metaData.numberOfEdgeSections++;
    return m_metaData.numberOfEdgeSections-1;
}

}
