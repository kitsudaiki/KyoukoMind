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
    : EdgeCluster(clusterId,
                  NODE_CLUSTER,
                  directoryPath)
{
    initNodeBlocks(numberOfNodes);
}

/**
 * @brief NodeCluster::NodeCluster
 * @param filePath
 */
NodeCluster::NodeCluster(const ClusterID clusterId,
                         const std::string directoryPath)
    : EdgeCluster(clusterId,
                  directoryPath)
{
    m_metaData.clusterType = NODE_CLUSTER;
}

/**
 * @brief NodeCluster::~NodeCluster
 */
NodeCluster::~NodeCluster()
{

}

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
 * @brief NodeCluster::addOutputCluster
 * @param clusterId
 * @param fakeAxonId
 * @param nodeId
 */
void NodeCluster::addOutputCluster(const ClusterID clusterId,
                                   const uint32_t fakeAxonId,
                                   const uint16_t nodeId)
{
    m_metaData.outgoing = 1;
    Neighbor neighbor;
    neighbor.neighborType = EDGE_CLUSTER;
    neighbor.targetClusterId = clusterId;
    neighbor.side = 15;
    addNeighbor(15, neighbor);
    getNodeBlock()[nodeId].targetAxonId = fakeAxonId;
    getNodeBlock()[nodeId].targetClusterPath = 15;
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
 * @brief NodeCluster::initNodeBlocks
 * @param numberOfNodes
 * @return
 */
bool NodeCluster::initNodeBlocks(uint16_t numberOfNodes)
{
    if(m_metaData.numberOfNodes == 0) {
        m_metaData.numberOfNodes = numberOfNodes;
        m_metaData.positionNodeBlocks = 1;

        uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
        m_metaData.numberOfNodeBlocks = (numberOfNodes * sizeof(KyoChanNode)) / blockSize + 1;

        m_clusterDataBuffer->allocateBlocks(m_metaData.numberOfNodeBlocks);
        updateMetaData(m_metaData);

        // fill array with empty nodes
        KyoChanNode* array = getNodeBlock();
        for(uint16_t i = 0; i < numberOfNodes; i++) {
            KyoChanNode tempNode;
            array[i] = tempNode;
        }
        m_clusterDataBuffer->syncAll();
        return true;
    }
    return false;
}


}
