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
 * @brief NodeCluster::addEdge
 * @param axonId
 * @param newEdge
 * @return
 */
bool NodeCluster::addEdge(const uint32_t axonId, const KyoChanEdge &newEdge)
{
    if(axonId >= m_metaData.numberOfAxons) {
        return false;
    }

    // get values
    KyoChanAxon* axon = &getAxonBlock()[axonId];
    uint32_t edgeSectionPos = axon->getLastEdgeSectionPos();
    KyoChanEdgeSection* edgeSection = &getEdgeBlock()[edgeSectionPos];

    // check if edge-section is full and add a new empty edge-section if necessary
    if(edgeSection->isFull()) {
        if(!addEmptyEdgeSection(axonId)) {
            return false;
        }
        // update values
        axon = &getAxonBlock()[axonId];
        edgeSectionPos = axon->getLastEdgeSectionPos();
        edgeSection = &getEdgeBlock()[edgeSectionPos];
    }

    // add the new edge to the edge-section
    edgeSection->addEdge(newEdge);

    return true;
}

/**
 * @brief NodeCluster::addEmptyEdgeSection
 * @param axonId
 * @return
 */
bool NodeCluster::addEmptyEdgeSection(const uint32_t axonId)
{
    // prechecks
    if(axonId >= m_metaData.numberOfAxons) {
        return false;
    }

    // add edge-section to axon
    KyoChanAxon* axon = &getAxonBlock()[axonId];
    if(!axon->addEdgeSectionPos(m_metaData.numberOfEdgeSections)) {
        return false;
    }

    // allocate a new block, if nesassary
    if(m_metaData.numberOfEdgeSections % 4 == 0) {
        if(!m_buffer->allocateBlocks(1)) {
            return false;
        }
        m_metaData.numberOfEdgeBlocks++;
    }

    // add new edge-section
    KyoChanEdgeSection newSection;
    getEdgeBlock()[m_metaData.numberOfEdgeSections] = newSection;
    m_metaData.numberOfEdgeSections++;

    return true;
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
            tempAxon.addEdgeSectionPos(i);
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

        // calculate number of edge-blocks
        uint32_t blockSize = m_buffer->getBlockSize();
        m_metaData.numberOfEdgeBlocks = (numberOfEdgeSections * sizeof(KyoChanEdgeSection)) / blockSize;
        if((numberOfEdgeSections * sizeof(KyoChanEdgeSection)) % blockSize != 0) {
            m_metaData.numberOfEdgeBlocks++;
        }

        // update and persist buffer
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
