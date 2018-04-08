/**
 *  @file    nodeCluster.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/cluster/nodeCluster.h>
#include <kyochanNetwork.h>
#include <files/ioBuffer.h>

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
                         const uint32_t numberOfNodes,
                         MessageController *controller)
    : EdgeCluster(clusterId,
                  directoryPath,
                  controller)
{
    m_clusterType = NODECLUSTER;

    initNodeBlocks(numberOfNodes);
}

/**
 * @brief NodeCluster::NodeCluster
 * @param filePath
 */
NodeCluster::NodeCluster(const ClusterID clusterId,
                         const std::string directoryPath,
                         MessageController *controller)
    : EdgeCluster(clusterId,
                  directoryPath,
                  controller)
{
    m_clusterType = NODECLUSTER;
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
 * @brief NodeCluster::getNodeBlock
 * @return
 */
KyoChanNode *NodeCluster::getNodeBlock()
{
    uint32_t positionNodeBlock = m_metaData.positionNodeBlocks;
    return (KyoChanNode*)m_buffer->getBlock(positionNodeBlock);
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

        uint32_t blockSize = m_buffer->getBlockSize();
        m_metaData.numberOfNodeBlocks = (numberOfNodes * sizeof(KyoChanNode)) / blockSize + 1;

        m_buffer->allocateBlocks(m_metaData.numberOfNodeBlocks);
        updateMetaData(m_metaData);

        // fill array with empty nodes
        KyoChanNode* array = getNodeBlock();
        for(uint16_t i = 0; i < numberOfNodes; i++) {
            KyoChanNode tempNode;
            array[i] = tempNode;
        }
        m_buffer->syncAll();
        return true;
    }
    return false;
}


}
