#include <core/cluster/nodeCluster.h>
#include <KyoChanNetwork.h>
#include <files/ioBuffer.h>
#include <QDebug>

namespace KyoukoMind
{

/**
 * @brief NodeCluster::NodeCluster
 * @param clusterId
 * @param directoryPath
 * @param numberOfNodes
 */
NodeCluster::NodeCluster(const ClusterID clusterId,
                         const QString directoryPath,
                         const quint32 numberOfNodes)
    : Cluster(clusterId,
              NODECLUSTER,
              directoryPath)
{
    m_metaData.numberOfNodes = numberOfNodes;
    m_metaData.numberOfNodes = numberOfNodes;

    quint32 blockSize = m_buffer->getBlockSize();
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
                         const QString directoryPath)
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
quint32 NodeCluster::getNumberOfNodeBlocks()
{
    return m_metaData.numberOfNodeBlocks;
}

/**
 * @brief NodeCluster::getNumberOfEdgeBlocks
 * @return
 */
quint32 NodeCluster::getNumberOfEdgeBlocks()
{
    return m_metaData.numberOfEdgeBlocks;
}

/**
 * @brief NodeCluster::getNodeBlock
 * @return
 */
KyoChanNode *NodeCluster::getNodeBlock()
{
    quint32 positionNodeBlock = m_metaData.positionNodeBlock;
    return (KyoChanNode*)m_buffer->getBlock(positionNodeBlock);
}

/**
 * @brief NodeCluster::getEdgeBlock
 * @return
 */
KyoChanEdgeSection *NodeCluster::getEdgeBlock()
{
    quint32 positionEdgeBlock = m_metaData.positionOfEdgeBlock;
    return (KyoChanEdgeSection*)m_buffer->getBlock(positionEdgeBlock);
}

/**
 * @brief NodeCluster::syncEdgeSections
 * @param startSection
 * @param endSection
 */
void NodeCluster::syncEdgeSections(quint32 startSection,
                                   quint32 endSection)
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
