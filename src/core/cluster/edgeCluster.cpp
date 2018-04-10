/**
 *  @file    edgeCluster.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/cluster/edgeCluster.h>
#include <kyochanNetwork.h>
#include <files/ioBuffer.h>

namespace KyoukoMind
{

/**
 * @brief EdgeCluster::EdgeCluster
 * @param clusterId
 * @param directoryPath
 * @param controller
 */
EdgeCluster::EdgeCluster(const ClusterID clusterId,
                         const std::string directoryPath,
                         MessageController *controller)
    : EmptyCluster(clusterId,
                   EDGECLUSTER,
                   directoryPath,
                   controller)
{
}

/**
 * @brief EdgeCluster::EdgeCluster
 * @param clusterId
 * @param clusterType
 * @param directoryPath
 * @param controller
 */
EdgeCluster::EdgeCluster(const ClusterID clusterId,
                         const uint8_t clusterType,
                         const std::string directoryPath,
                         MessageController *controller)
    : EmptyCluster(clusterId,
                   clusterType,
                   directoryPath,
                   controller)
{
}

/**
 * @brief EdgeCluster::~EdgeCluster
 */
EdgeCluster::~EdgeCluster()
{

}

/**
 * @brief EdgeCluster::getNumberOfAxonBlocks
 * @return
 */
uint32_t EdgeCluster::getNumberOfAxonBlocks() const
{
    return m_metaData.numberOfAxonBlocks;
}

/**
 * @brief EdgeCluster::getNumberOfEdgeBlocks
 * @return
 */
uint32_t EdgeCluster::getNumberOfEdgeBlocks() const
{
    return m_metaData.numberOfEdgeBlocks;
}


/**
 * @brief EdgeCluster::getAxonBlock
 * @return
 */
KyoChanAxon *EdgeCluster::getAxonBlock()
{
    uint32_t positionAxonBlock = m_metaData.positionAxonBlocks;
    return (KyoChanAxon*)m_buffer->getBlock(positionAxonBlock);
}

/**
 * @brief EdgeCluster::getEdgeBlock
 * @return
 */
KyoChanEdgeSection *EdgeCluster::getEdgeBlock()
{
    uint32_t positionEdgeBlock = m_metaData.positionOfEdgeBlock;
    return (KyoChanEdgeSection*)m_buffer->getBlock(positionEdgeBlock);
}

/**
 * @brief EdgeCluster::initAxonBlocks
 * @param numberOfAxons
 * @return
 */
bool EdgeCluster::initAxonBlocks(uint32_t numberOfAxons)
{
    if(m_metaData.numberOfAxons == 0) {
        m_metaData.numberOfAxons = numberOfAxons;
        m_metaData.positionAxonBlocks = m_metaData.positionNodeBlocks + m_metaData.numberOfNodeBlocks;

        uint32_t blockSize = m_buffer->getBlockSize();
        m_metaData.numberOfAxonBlocks = (numberOfAxons * sizeof(KyoChanAxon)) / blockSize;
        if((numberOfAxons * sizeof(KyoChanAxon)) % blockSize != 0) {
            m_metaData.numberOfAxonBlocks += 1;
        }

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
 * @brief EdgeCluster::initEdgeBlocks
 * @param numberOfEdgeSections
 * @return
 */
bool EdgeCluster::initEdgeBlocks(uint32_t numberOfEdgeSections)
{
    if(m_metaData.numberOfEdgeSections == 0) {
        m_metaData.numberOfEdgeSections = numberOfEdgeSections;
        m_metaData.positionOfEdgeBlock = m_metaData.positionAxonBlocks + m_metaData.numberOfAxonBlocks;

        // calculate number of edge-blocks
        uint32_t blockSize = m_buffer->getBlockSize();
        m_metaData.numberOfEdgeBlocks = (numberOfEdgeSections * sizeof(KyoChanEdgeSection)) / blockSize;
        if((numberOfEdgeSections * sizeof(KyoChanEdgeSection)) % blockSize != 0) {
            m_metaData.numberOfEdgeBlocks += 1;
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

/**
 * @brief EdgeCluster::addEdge
 * @param axonId
 * @param newEdge
 * @return
 */
bool EdgeCluster::addEdge(const uint32_t axonId, const KyoChanEdge &newEdge)
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
 * @brief EdgeCluster::addEmptyEdgeSection
 * @param axonId
 * @return
 */
bool EdgeCluster::addEmptyEdgeSection(const uint32_t axonId)
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
 * @brief EdgeCluster::syncEdgeSections
 * @param startSection
 * @param endSection
 */
void EdgeCluster::syncEdgeSections(uint32_t startSection,
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
