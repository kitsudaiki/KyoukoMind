#include <core/cluster/nodeCluster.h>
#include <KyoChanNetwork.h>
#include <files/ioBuffer.h>
#include <QDebug>

namespace KyoChan_Network
{

/**
 * @brief NodeCluster::NodeCluster
 * @param clusterId
 * @param numberOfNodes
 * @param parentCluster
 */
NodeCluster::NodeCluster(quint32 clusterId,
                         quint32 numberOfNodes,
                         QString directoryPath,
                         Cluster* parentCluster)
    : Cluster(clusterId,
              NODECLUSTER,
              parentCluster)
{
    // TODO
    QString path = "/tmp/poi" + QString::number(clusterId);
    m_buffer = new Persistence::IOBuffer(path);
    m_buffer->allocateBlocks(1);

    getMetaData()->numberOfNodes = numberOfNodes;
    getMetaData()->numberOfNodes = numberOfNodes;
    getMetaData()->numberOfNodeBlocks = (numberOfNodes/4) + 1;
    getMetaData()->numberOfEdgeBlocks = (numberOfNodes/4) + 1;

    m_buffer->allocateBlocks(getMetaData()->numberOfNodeBlocks
                             + getMetaData()->numberOfEdgeBlocks);

    /*for(int i = 0; i < 7; i++) {
        m_borderConnections[i] = KyoChanNetwork::m_mq->createClient(clusterId,
                                                                    i);
    }*/
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
    return getMetaData()->numberOfNodeBlocks;
}

/**
 * @brief NodeCluster::getNumberOfEdgeBlocks
 * @return
 */
quint32 NodeCluster::getNumberOfEdgeBlocks()
{
    return getMetaData()->numberOfEdgeBlocks;
}

/**
 * @brief NodeCluster::getNodeBlock
 * @return
 */
KyoChanNode *NodeCluster::getNodeBlock()
{
    quint32 positionNodeBlock = getMetaData()->positionNodeBlock;
    return (KyoChanNode*)m_buffer->getBlock(positionNodeBlock);
}

/**
 * @brief NodeCluster::getEdgeBlock
 * @return
 */
KyoChanEdgeSection *NodeCluster::getEdgeBlock()
{
    quint32 positionEdgeBlock = getMetaData()->positionOfEdgeBlock;
    return (KyoChanEdgeSection*)m_buffer->getBlock(positionEdgeBlock);
}

/**
 * @brief NodeCluster::syncEdgeSections
 * @param startSection
 * @param endSection
 */
void NodeCluster::syncEdgeSections(quint32 startSection, quint32 endSection)
{
    if(endSection >= getNumberOfEdgeBlocks()){
        endSection = getNumberOfEdgeBlocks() - 1;
    }
    if(endSection < startSection) {
        startSection = 0;
    }
    startSection += getMetaData()->positionOfEdgeBlock;
    endSection += getMetaData()->positionOfEdgeBlock;
    m_buffer->syncBlocks(startSection, endSection);
}

}
