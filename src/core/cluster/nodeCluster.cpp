#include <core/cluster/nodeCluster.h>
#include <KyoChanNetwork.h>
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
                         const uint32_t numberOfNodes)
    : Cluster(clusterId,
              NODECLUSTER,
              directoryPath)
{
    m_metaData.numberOfNodes = numberOfNodes;
    m_metaData.numberOfNodes = numberOfNodes;

    uint32_t blockSize = m_buffer->getBlockSize();
    m_metaData.numberOfNodeBlocks = (numberOfNodes * sizeof(KyoChanNode)) / blockSize + 1;
    m_metaData.numberOfEdgeBlocks = numberOfNodes;

    m_buffer->allocateBlocks(m_metaData.numberOfNodeBlocks
                             + m_metaData.numberOfEdgeBlocks);
    updateMetaData(m_metaData);
}

/**
 * @brief NodeCluster::NodeCluster
 * @param filePath
 */
NodeCluster::NodeCluster(const ClusterID clusterId,
                         const std::string directoryPath)
    : Cluster(clusterId,
              NODECLUSTER,
              directoryPath)
{
}

/**
 * @brief NodeCluster::~NodeCluster
 */
NodeCluster::~NodeCluster()
{
    if(m_buffer != nullptr) {
        m_buffer->closeBuffer();
        m_buffer = nullptr;
    }
}

/**
 * @brief NodeCluster::getNumberOfNode
 * @return
 */
uint32_t NodeCluster::getNumberOfNodeBlocks()
{
    return m_metaData.numberOfNodeBlocks;
}

/**
 * @brief NodeCluster::getNumberOfEdgeBlocks
 * @return
 */
uint32_t NodeCluster::getNumberOfEdgeBlocks()
{
    return m_metaData.numberOfEdgeBlocks;
}

/**
 * @brief NodeCluster::getNodeBlock
 * @return
 */
KyoChanNode *NodeCluster::getNodeBlock()
{
    uint32_t positionNodeBlock = m_metaData.positionNodeBlock;
    return (KyoChanNode*)m_buffer->getBlock(positionNodeBlock);
}

/**
 * @brief NodeCluster::getEdgeBlock
 * @return
 */
KyoChanEdgeSection *NodeCluster::getEdgeBlock()
{
    uint32_t positionEdgeBlock = m_metaData.positionOfEdgeBlock;
    return (KyoChanEdgeSection*)m_buffer->getBlock(positionEdgeBlock);
}

/**
 * @brief NodeCluster::syncEdgeSections
 * @param startSection
 * @param endSection
 */
void NodeCluster::syncEdgeSections(uint32_t startSection,
                                   uint32_t endSection)
{
    if(endSection >= getNumberOfEdgeBlocks()){
        endSection = getNumberOfEdgeBlocks() - 1;
    }
    if(endSection < startSection) {
        startSection = 0;
    }
    startSection += m_metaData.positionOfEdgeBlock;
    endSection += m_metaData.positionOfEdgeBlock;
    m_buffer->syncBlocks(startSection, endSection);
}

}
