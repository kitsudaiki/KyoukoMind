/**
 *  @file    edgeCluster.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/clustering/cluster/edgeCluster.h>
#include <kyochanNetwork.h>
#include <files/dataBuffer.h>
#include <core/structs/kyochanEdges.h>

namespace KyoukoMind
{

/**
 * @brief EdgeCluster::EdgeCluster
 * @param clusterId
 * @param directoryPath
 */
EdgeCluster::EdgeCluster(const ClusterID clusterId,
                         const std::string directoryPath)
    : Cluster(clusterId,
              EDGE_CLUSTER,
              directoryPath)
{
}

/**
 * @brief EdgeCluster::~EdgeCluster
 */
EdgeCluster::~EdgeCluster()
{}

/**
 * @brief EdgeCluster::getNumberOfEdgeBlocks
 * @return
 */
uint32_t EdgeCluster::getNumberOfEdgeBlocks() const
{
    return m_metaData.numberOfEdgeBlocks;
}

/**
 * @brief EdgeCluster::getEdgeBlock
 * @return
 */
KyoChanEdgeForwardSection *EdgeCluster::getEdgeBlock()
{
    uint32_t positionEdgeBlock = m_metaData.positionOfEdgeBlock;
    return (KyoChanEdgeForwardSection*)m_clusterDataBuffer->getBlock(positionEdgeBlock);
}

/**
 * @brief EdgeCluster::initEdgeBlocks
 * @param numberOfEdgeSections
 * @return
 */
bool EdgeCluster::initEdgeBlocks(const uint32_t numberOfEdgeSections)
{
    if(m_metaData.numberOfEdgeSections != 0 || numberOfEdgeSections == 0) {
        return false;
    }

    m_metaData.numberOfEdgeSections = numberOfEdgeSections;
    m_metaData.positionOfEdgeBlock = m_metaData.positionAxonBlocks + m_metaData.numberOfAxonBlocks;

    // calculate number of edge-blocks
    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    m_metaData.numberOfEdgeBlocks = (numberOfEdgeSections * sizeof(KyoChanEdgeForwardSection)) / blockSize;
    if((numberOfEdgeSections * sizeof(KyoChanEdgeForwardSection)) % blockSize != 0) {
        m_metaData.numberOfEdgeBlocks += 1;
    }

    // update and persist buffer
    m_clusterDataBuffer->allocateBlocks(m_metaData.numberOfEdgeBlocks);
    updateMetaData(m_metaData);

    // fill array with empty edgesections
    KyoChanEdgeForwardSection* array = getEdgeBlock();
    for(uint32_t i = 0; i < numberOfEdgeSections; i++)
    {
        KyoChanEdgeForwardSection newSection;
        array[i] = newSection;
    }
    m_clusterDataBuffer->syncAll();
    return true;
}

/**
 * @brief EdgeCluster::addEmptyEdgeForwardSection
 * @param marker
 * @return
 */
uint32_t EdgeCluster::addEmptyEdgeSection(const uint32_t marker)
{
    // allocate a new block, if necessary
    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    if((m_metaData.numberOfEdgeSections * sizeof(KyoChanEdgeForwardSection) )/ blockSize
            < ((m_metaData.numberOfEdgeSections + 1) * sizeof(KyoChanEdgeForwardSection)) / blockSize)
    {
        // TODO: outsourcing
        if(!m_clusterDataBuffer->allocateBlocks(1)) {
            return 0;
        }
        m_metaData.numberOfEdgeBlocks++;
    }

    // add new edge-forward-section
    KyoChanEdgeForwardSection newSection;
    newSection.marker = marker;
    getEdgeBlock()[m_metaData.numberOfEdgeSections] = newSection;
    m_metaData.numberOfEdgeSections++;

    return m_metaData.numberOfEdgeSections-1;
}

}
