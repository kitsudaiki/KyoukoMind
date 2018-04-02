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
    if(m_buffer != nullptr) {
        m_buffer->closeBuffer();
        delete m_buffer;
        m_buffer = nullptr;
    }
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
 * @brief NodeCluster::getNumberOfAxonBlocks
 * @return
 */
uint32_t NodeCluster::getNumberOfAxonBlocks() const
{
    return m_metaData.numberOfAxonBlocks;
}

/**
 * @brief NodeCluster::getNumberOfEdgeBlocks
 * @return
 */
uint32_t NodeCluster::getNumberOfEdgeBlocks() const
{
    return m_metaData.numberOfEdgeBlocks;
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
 * @brief NodeCluster::getAxonBlock
 * @return
 */
KyoChanAxon *NodeCluster::getAxonBlock()
{
    uint32_t positionAxonBlock = m_metaData.positionAxonBlocks;
    return (KyoChanAxon*)m_buffer->getBlock(positionAxonBlock);
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

/**
 * @brief NodeCluster::addEmptyEdgeSection
 * @param axonId
 * @return
 */
bool NodeCluster::addEmptyEdgeSection(const uint32_t axonId)
{
    if(axonId >= m_metaData.numberOfAxons) {
        return false;
    }
    if(getAxonBlock()[axonId].numberOfEdgeSections == 99) {
        return false;
    }

    // update values in axon
    uint8_t pos = getAxonBlock()[axonId].numberOfEdgeSections;
    getAxonBlock()[axonId].edgeSections[pos] = m_metaData.numberOfEdgeSections;

    // allocate a new block, if nesassary
    if(m_metaData.numberOfEdgeSections % 4 == 0) {
        m_buffer->allocateBlocks(1);
    }

    // add new section
    KyoChanEdgeSection newSection;
    getEdgeBlock()[m_metaData.numberOfEdgeSections] = newSection;

    // update counter
    m_metaData.numberOfEdgeSections++;
    getAxonBlock()[axonId].numberOfEdgeSections++;
    return true;
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

/**
 * @brief NodeCluster::initAxonBlocks
 * @param numberOfAxons
 * @return
 */
bool NodeCluster::initAxonBlocks(uint32_t numberOfAxons)
{
    if(m_metaData.numberOfAxons == 0) {
        m_metaData.numberOfAxons = numberOfAxons;
        m_metaData.positionAxonBlocks = m_metaData.positionNodeBlocks + m_metaData.numberOfNodeBlocks;

        uint32_t blockSize = m_buffer->getBlockSize();
        m_metaData.numberOfAxonBlocks = (numberOfAxons * sizeof(KyoChanAxon)) / blockSize + 1;

        m_buffer->allocateBlocks(m_metaData.numberOfAxonBlocks);
        updateMetaData(m_metaData);

        initEdgeBlocks(numberOfAxons);

        // fill array with empty axons
        KyoChanAxon* array = getAxonBlock();
        for(uint32_t i = 0; i < numberOfAxons; i++) {
            KyoChanAxon tempAxon;
            array[i] = tempAxon;
        }
        m_buffer->syncAll();
        return true;
    }
    return false;
}

/**
 * @brief NodeCluster::initEdgeBlocks
 * @param numberOfEdgeSections
 * @return
 */
bool NodeCluster::initEdgeBlocks(uint32_t numberOfEdgeSections)
{
    if(m_metaData.numberOfEdgeSections == 0) {
        m_metaData.numberOfEdgeSections = numberOfEdgeSections;
        m_metaData.positionOfEdgeBlock = m_metaData.positionAxonBlocks + m_metaData.numberOfAxonBlocks;

        uint32_t blockSize = m_buffer->getBlockSize();
        m_metaData.numberOfEdgeBlocks = (numberOfEdgeSections * sizeof(KyoChanEdgeSection)) / blockSize + 1;

        m_buffer->allocateBlocks(m_metaData.numberOfEdgeBlocks);
        updateMetaData(m_metaData);

        // fill array with empty edgesections
        KyoChanEdgeSection* array = getEdgeBlock();
        for(uint32_t i = 0; i < numberOfEdgeSections; i++) {
            KyoChanEdgeSection tempNode;
            array[i] = tempNode;
        }
        m_buffer->syncAll();
        return true;
    }
    return false;
}

}
