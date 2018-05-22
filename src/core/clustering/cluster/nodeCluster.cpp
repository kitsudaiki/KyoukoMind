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
 * @brief NodeCluster::NodeCluster constructor for node-cluster-creation
 * @param clusterId id for the new cluster
 * @param directoryPath path to the directory, where the file of the cluster should be saved
 * @param numberOfNodes number of nodes of the new node-cluster
 */
NodeCluster::NodeCluster(const ClusterID clusterId,
                         const std::string directoryPath,
                         const uint32_t numberOfNodes)
    : EdgeCluster(clusterId,
                  directoryPath)
{
    m_metaData.clusterType = NODE_CLUSTER;
    initNodeBlocks(numberOfNodes);
    // TODO: readd file-path after tests
    m_edgeSectionBuffer = new PerformanceIO::DataBuffer("");
    m_edgeSectionBuffer->allocateBlocks(1);
}

/**
 * @brief NodeCluster::~NodeCluster destroy edge-section-buffer
 */
NodeCluster::~NodeCluster()
{
    if(m_edgeSectionBuffer != nullptr)
    {
        m_edgeSectionBuffer->closeBuffer();
        delete m_edgeSectionBuffer;
        m_edgeSectionBuffer = nullptr;
    }
}

/**
 * @brief NodeCluster::getNumberOfNodeBlocks get number of node-blocks of the cluster in the buffer
 * @return number of node-blocks
 */
uint16_t NodeCluster::getNumberOfNodeBlocks() const
{
    return m_metaData.numberOfNodeBlocks;
}

/**
 * @brief NodeCluster::getNumberOfNode get number of nodes of the cluster
 * @return number of nodes
 */
uint16_t NodeCluster::getNumberOfNodes() const
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
bool NodeCluster::initNodeBlocks(const uint16_t numberOfNodes)
{
    // prechecks
    if(numberOfNodes == 0 || m_metaData.numberOfNodes != 0) {
        return false;
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

    // write the new init nodes to the buffer and the file
    m_clusterDataBuffer->syncAll();
    return true;
}

/**
 * @brief NodeCluster::getEdgeSectionBlock
 * @return
 */
KyoChanEdgeSection *NodeCluster::getEdgeSectionBlock()
{
    uint32_t positionEdgeBlock = m_metaData.positionOfEdgeBlock;
    return (KyoChanEdgeSection*)m_edgeSectionBuffer->getBlock(positionEdgeBlock);
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
    m_metaData.positionOfEdgeBlock = 0;

    // calculate number of edge-blocks
    uint32_t blockSize = m_edgeSectionBuffer->getBlockSize();
    m_metaData.numberOfEdgeBlocks = (numberOfEdgeSections * sizeof(KyoChanEdgeSection)) / blockSize;
    if((numberOfEdgeSections * sizeof(KyoChanEdgeSection)) % blockSize != 0) {
        m_metaData.numberOfEdgeBlocks += 1;
    }

    // update and persist buffer
    m_edgeSectionBuffer->allocateBlocks(m_metaData.numberOfEdgeBlocks);
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
    m_edgeSectionBuffer->syncAll();
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
 */
uint32_t NodeCluster::addEmptyEdgeSection()
{
    // allocate a new block, if necessary
    uint32_t blockSize = m_edgeSectionBuffer->getBlockSize();
    if((m_metaData.numberOfEdgeSections * sizeof(KyoChanEdgeSection) )/ blockSize
            < ((m_metaData.numberOfEdgeSections + 1) * sizeof(KyoChanEdgeSection)) / blockSize)
    {
        if(!m_edgeSectionBuffer->allocateBlocks(1)) {
            return SPECIAL_STATE;
        }
        m_metaData.numberOfEdgeBlocks++;
    }

    // add new edge-forward-section
    KyoChanEdgeSection newSection;
    getEdgeSectionBlock()[m_metaData.numberOfEdgeSections] = newSection;
    m_metaData.numberOfEdgeSections++;
    return m_metaData.numberOfEdgeSections-1;
}

}
