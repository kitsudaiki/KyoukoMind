/**
 *  @file    cluster.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "cluster.h"

#include <commonDataBuffer.h>
#include <kyoChanNetwork.h>
#include <core/clustering/globalValuesHandler.h>

#include <core/messaging/incomingMessageBuffer.h>
#include <core/messaging/outgoingMessageBuffer.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>

#include <tests/core/clustering/cluster/clusterTest.h>

namespace KyoukoMind
{

/**
 * @brief Cluster::Cluster common cluster-constructor
 * @param clusterId id of the cluster
 * @param staticItemSize
 * @param directoryPath path to the directory with the persisted cluster-file
 * @param dynamicItemSize
 */
Cluster::Cluster(const ClusterID &clusterId,
                 const uint32_t x,
                 const uint32_t y,
                 const std::string directoryPath,
                 const uint32_t staticItemSize,
                 const uint32_t dynamicItemSize) :
    m_staticItemSize(staticItemSize),
    m_dynamicItemSize(dynamicItemSize)

{
    m_metaData.clusterId = clusterId;
    m_metaData.clusterPos.x = x;
    m_metaData.clusterPos.y = y;

    // creates a file for the cluster with buffer
    // TODO: readd file-path after tests
    std::string filePath = directoryPath
                         + "/cluster_" + std::to_string(clusterId);
    // TODO: readd persistency
    m_clusterDataBuffer = new Kitsune::CommonDataBuffer();

    // init the buffer and the file with the first block for the meta-data
    m_clusterDataBuffer->allocateBlocks(1);
    updateMetaData();

    // init empty neighbor-list to avoid not initialized values
    for(uint8_t side = 0; side < 17; side++)
    {
        Neighbor emptyNeighbor;
        emptyNeighbor.targetClusterId = UNINIT_STATE_32;
        emptyNeighbor.targetSide = 16 - side;
        m_metaData.neighors[side] = emptyNeighbor;
    }
}

/**
 * @brief Cluster::~Cluster common destructor for cluster
 */
Cluster::~Cluster()
{
    // delete the main-buffer-object if still exist
    if(m_clusterDataBuffer != nullptr)
    {
        delete m_clusterDataBuffer;
        m_clusterDataBuffer = nullptr;
    }
}


/**
 * @brief Cluster::getMetaData get the meta-data of the cluster
 * @return meta-data-object with all common cluster-informations
 */
ClusterMetaData
Cluster::getMetaData() const
{
    return m_metaData;
}

/**
 * @brief Cluster::getMetaDataFromBuffer reads the meta-data from the file-buffer
 */
void
Cluster::getMetaDataFromBuffer()
{
    ClusterMetaData* metaDataPointer = (ClusterMetaData*)m_clusterDataBuffer->getBlock(0);
    m_metaData = *metaDataPointer;
}

/**
 * @brief Cluster::updateMetaData write the current mata-data to the buffer and the file
 */
void
Cluster::updateMetaData()
{
    uint32_t size = sizeof(ClusterMetaData);
    void* metaDataBlock = m_clusterDataBuffer->getBlock(0);
    memcpy(metaDataBlock, &m_metaData, size);
    // TODO: readd persist meta-data-changes
    // m_clusterDataBuffer->syncBlocks(0, 0);
}

/**
 * @brief Cluster::getClusterId returns the cluster-id from the meta-data
 * @return id of the cluster
 */
ClusterID
Cluster::getClusterId() const
{
    return m_metaData.clusterId;
}

/**
 * @brief Cluster::getClusterType returns the cluster-type from the meta-data
 * @return type of the cluster as enum
 */
uint8_t
Cluster::getClusterType() const
{
    return m_metaData.clusterType;
}

/**
 * @brief Cluster::getIncomingMessageBuffer get the incoming message-buffer
 * @return pointer to the incoming message-buffer
 */
IncomingMessageBuffer*
Cluster::getIncomingMessageBuffer(const uint8_t side)
{
    if(side < 17) {
        return m_metaData.neighors[side].incomBuffer;
    }
    return nullptr;
}

/**
 * @brief Cluster::getOutgoingMessageBuffer get the outgoing message-buffer
 * @return pointer to the outgoing message-buffer
 */
OutgoingMessageBuffer*
Cluster::getOutgoingMessageBuffer(const uint8_t side)
{
    if(side < 17) {
        return m_metaData.neighors[side].outgoBuffer;
    }
    return nullptr;
}

/**
 * @brief Cluster::setIncomingMessageBuffer add new already connected incoming message-buffer
 * @param side side-id where the buffer should be added
 * @param buffer pointer to the message-buffer which should be added
 * @return false if buffer or side are invalid, else true
 */
bool
Cluster::setNewConnection(const uint8_t side,
                          IncomingMessageBuffer* buffer)
{
    if(side < 17 && buffer != nullptr)
    {
        m_metaData.neighors[side].outgoBuffer->setIncomingBuffer(buffer);
        return true;
    }
    return false;
}

/**
 * @brief Cluster::isBufferReady check if all incoming message-buffers of the cluster are ready for processing
 * @return true if all message-buffer of the cluster have an incoming message, else false
 */
bool
Cluster::isBufferReady()
{
    bool result = true;
    // iterate over all neighbors
    for(uint8_t side = 1; side < 16; side++)
    {
        IncomingMessageBuffer* incomBuffer = m_metaData.neighors[side].incomBuffer;
        if(incomBuffer != nullptr) {
            result = result && m_metaData.neighors[side].incomBuffer->isReady();
        }
    }
    return result;
}

/**
 * @brief Cluster::setNeighbor set a new neighbor to the cluster
 * @param side side for the new neighbor
 * @param targetClusterId id of the neighbor-target
 * @return true, if neighbor was added, else false
 */
bool
Cluster::setNeighbor(const uint8_t side, const ClusterID targetClusterId)
{
    if(side == 16) {
        m_metaData.outputCluster = 1;
    }

    if(side == 0) {
        m_metaData.inputCluster = 1;
    }

    // check if side is valid
    if(side < 17)
    {
        // add the new neighbor
        m_metaData.neighors[side].targetClusterId = targetClusterId;
        m_metaData.neighors[side].targetSide = 16 - side;
        if(m_metaData.neighors[side].incomBuffer != nullptr) {
           delete m_metaData.neighors[side].incomBuffer;
        }
        m_metaData.neighors[side].incomBuffer = new IncomingMessageBuffer();
        if(m_metaData.neighors[side].outgoBuffer != nullptr) {
           delete m_metaData.neighors[side].outgoBuffer;
        }
        m_metaData.neighors[side].outgoBuffer = new OutgoingMessageBuffer();

        // persist meta-data
        updateMetaData();
        return true;
    }
    return false;
}

/**
 * @brief Cluster::getNeighbors get the list with the neighbors of the cluster
 * @return pointer to the beginning of the neighbor-list
 */
Neighbor*
Cluster::getNeighbors()
{
    return &m_metaData.neighors[0];
}

/**
 * @brief Cluster::getNeighborId get the id of a special neighbor
 * @param side side of the requested neighbor
 * @return id of the neighbor, or uninit value if side is invalid
 */
ClusterID
Cluster::getNeighborId(const uint8_t side)
{
    if(side < 17) {
        return m_metaData.neighors[side].targetClusterId;
    }
    return UNINIT_STATE_32;
}

/**
 * @brief Cluster::initCycle
 * @return
 */
bool
Cluster::initCycle()
{
    if(m_cycleInProgress == true) {
        return false;
    }

    m_cycleInProgress = true;
    m_numberOfIncomingMessages = 0;
    m_neighborInfo.reset();

    m_globalValue = m_globalValuesHandler->getGlobalValues();
    for(uint8_t side = 0; side < 17; side++)
    {
        IncomingMessageBuffer* incomBuffer = m_metaData.neighors[side].incomBuffer;
        if(incomBuffer != nullptr)
        {
            // get and check message
            DataMessage* currentMessage = incomBuffer->getMessage();
            if(currentMessage == nullptr) {
                continue;
            }

            m_neighborInfo.addValues(currentMessage->getMetaData().neighborInfos);

            m_incomingMessages[m_numberOfIncomingMessages] = std::make_pair(currentMessage, side);
            m_numberOfIncomingMessages++;
        }
    }
    return true;
}

/**
 * @brief Cluster::finishCycle finish the current cycle with sending messages from the outgoing buffer
 */
void
Cluster::finishCycle(NeighborInformation *externalInfo)
{
    // TODO. remove externalInfo because its only for testing
    if(externalInfo != nullptr) {
        m_neighborInfo = *externalInfo;
    } else {
        m_neighborInfo.localMemorizing /= MEMORIZING_COOLDOWN;
        m_neighborInfo.localLearing /= LEARNING_COOLDOWN;
    }
    for(uint8_t side = 0; side < 17; side++)
    {
        if(m_metaData.neighors[side].incomBuffer != nullptr
                && m_metaData.neighors[side].outgoBuffer != nullptr)
        {
            m_metaData.neighors[side].incomBuffer->finish();
            m_metaData.neighors[side].outgoBuffer->finishCycle(
                        m_metaData.neighors[side].targetClusterId,
                        16 - side,
                        m_metaData.clusterId,
                        m_neighborInfo);
        }
    }

    reportStatus();

    m_cycleInProgress = false;
}

/**
 * @brief Cluster::reportStatus
 */
void
Cluster::reportStatus()
{
    // construct metadata for monitoring
    MonitoringMetaData metaDataMonitoring;
    metaDataMonitoring.numberOfStaticItems = m_metaData.numberOfStaticItems;
    metaDataMonitoring.numberOfDynamicItems = m_metaData.numberOfDynamicItems;
    metaDataMonitoring.numberOfDeletedDynamicItems = m_metaData.numberOfDeletedDynamicItems;
    metaDataMonitoring.numberOfStaticBlocks = m_metaData.numberOfStaticBlocks;
    metaDataMonitoring.numberOfDynamicBlocks = m_metaData.numberOfDynamicBlocks;

    m_globalValue = m_globalValuesHandler->getGlobalValues();
    metaDataMonitoring.globalLearning = m_globalValue.globalLearningOffset;
    metaDataMonitoring.globalMemorizing = m_globalValue.globalMemorizingOffset;

    m_processingData.averagetAxonPotential /= m_processingData.numberOfActiveAxons;
    m_processingData.averagetNodePotential /= m_processingData.numberOfNodes;

    KyoukoNetwork::m_reporter->sendStatus(m_metaData.clusterId,
                                          m_metaData.clusterPos.x,
                                          m_metaData.clusterPos.y,
                                          m_metaData.clusterType,
                                          metaDataMonitoring,
                                          m_processingData);
    m_processingData.reset();
}

/**
 * @brief Cluster::isCycleInProgress
 * @return
 */
bool
Cluster::isCycleInProgress() const
{
    return m_cycleInProgress;
}

/**
 * @brief Cluster::setGlobalValuesHandler
 * @param globalValuesHandler
 */
void
Cluster::setGlobalValuesHandler(GlobalValuesHandler* globalValuesHandler)
{
    m_globalValuesHandler = globalValuesHandler;
}

/**
 * @brief Cluster::setLearningOverride
 * @param value
 */
void Cluster::setLearningOverride(float value)
{
    m_learningOverride = value;
}

/**
 * @brief Cluster::isInputCluster
 * @return
 */
bool Cluster::isInputCluster()
{
    if(m_metaData.inputCluster == 0) {
        return false;
    }
    return true;
}

/**
 * @brief Cluster::initStaticBlocks initialize the node-list of the cluster
 * @param numberOfItems number of new empty items
 * @return false if nodes are already initialized, esle true
 */
bool
Cluster::initStaticBlocks(const uint32_t numberOfItems)
{
    // prechecks
    if(m_metaData.numberOfStaticItems != 0
            || m_staticItemSize == 0) {
        return false;
    }

    // update meta-data of the cluster
    m_metaData.numberOfStaticItems = numberOfItems;
    m_metaData.positionOfStaticBlocks = 1;

    const uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    m_metaData.numberOfStaticBlocks = (numberOfItems * m_staticItemSize) / blockSize + 1;

    // allocate blocks in buffer
    m_clusterDataBuffer->allocateBlocks(m_metaData.numberOfStaticBlocks);
    updateMetaData();

    m_metaData.positionOfDynamicBlocks = m_metaData.positionOfStaticBlocks + m_metaData.numberOfStaticBlocks;

    return true;
}

/**
 * @brief Cluster::initDynamicBlocks initialize forward-edge-block
 * @param numberOfItems number of forward-edge-sections
 * @return true if success, else false
 */
bool
Cluster::initDynamicBlocks(const uint32_t numberOfItems)
{
    // prechecks
    if(m_metaData.numberOfDynamicItems != 0
            || numberOfItems == 0
            || m_dynamicItemSize == 0) {
        return false;
    }

    // update meta-data of the cluster
    m_metaData.numberOfDynamicItems = numberOfItems;
    m_metaData.positionOfDynamicBlocks = m_metaData.numberOfStaticBlocks + 1;

    // calculate number of edge-blocks
    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    m_metaData.numberOfDynamicBlocks= (numberOfItems * m_dynamicItemSize) / blockSize;
    if((numberOfItems * m_dynamicItemSize) % blockSize != 0) {
        m_metaData.numberOfDynamicBlocks += 1;
    }

    // update and persist buffer
    m_clusterDataBuffer->allocateBlocks(m_metaData.numberOfDynamicBlocks);
    updateMetaData();

    return true;
}

/**
 * @brief Cluster::reserveDynamicItem add a new forward-edge-section
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
uint32_t
Cluster::reserveDynamicItem()
{
    if(m_dynamicItemSize == 0) {
        return UNINIT_STATE_32;
    }

    // try to reuse item
    const uint32_t reusePos = reuseItemPosition();
    if(reusePos != UNINIT_STATE_32) {
        return reusePos;
    }

    // allocate a new block, if necessary
    uint32_t blockSize = m_clusterDataBuffer->getBlockSize();
    if((m_metaData.numberOfDynamicItems * m_dynamicItemSize)/ blockSize
            < ((m_metaData.numberOfDynamicItems + 1) * m_dynamicItemSize) / blockSize)
    {
        if(!m_clusterDataBuffer->allocateBlocks(1)) {
            return UNINIT_STATE_32;
        }
        m_metaData.numberOfDynamicBlocks++;
    }

    m_metaData.numberOfDynamicItems++;
    return m_metaData.numberOfDynamicItems-1;
}

/**
 * @brief Cluster::reuseItemPosition
 * @return
 */
uint32_t
Cluster::reuseItemPosition()
{
    const uint32_t selectedPosition = m_metaData.bytePositionOfFirstEmptyBlock;
    if(selectedPosition == UNINIT_STATE_32) {
        return UNINIT_STATE_32;
    }

    uint8_t* blockBegin = m_clusterDataBuffer->getBlock(m_metaData.positionOfDynamicBlocks);
    EmptyPlaceHolder* secetedPlaceHolder = (EmptyPlaceHolder*)&blockBegin[selectedPosition];
    m_metaData.bytePositionOfFirstEmptyBlock = secetedPlaceHolder->bytePositionOfNextEmptyBlock;

    if(m_metaData.bytePositionOfFirstEmptyBlock == UNINIT_STATE_32) {
        m_metaData.bytePositionOfLastEmptyBlock = UNINIT_STATE_32;
    }

    m_metaData.numberOfDeletedDynamicItems--;
    assert(selectedPosition % m_dynamicItemSize == 0);
    return selectedPosition / m_dynamicItemSize;
}

/**
 * @brief Cluster::deleteDynamicItem
 * @param itemPos
 * @return
 */
bool
Cluster::deleteDynamicItem(const uint32_t itemPos)
{
    if(itemPos >= m_metaData.numberOfDynamicItems) {
        return false;
    }

    // get buffer
    uint8_t* blockBegin = m_clusterDataBuffer->getBlock(m_metaData.positionOfDynamicBlocks);

    // overwrite item
    const uint32_t currentBytePos = itemPos * m_dynamicItemSize;
    EmptyPlaceHolder* placeHolder = (EmptyPlaceHolder*)&blockBegin[currentBytePos];

    if(placeHolder->status == DELETED_SECTION) {
        return false;
    }

    placeHolder->bytePositionOfNextEmptyBlock = UNINIT_STATE_32;
    placeHolder->status = DELETED_SECTION;

    // modify last place-holder
    if(m_metaData.bytePositionOfLastEmptyBlock != UNINIT_STATE_32)
    {
        EmptyPlaceHolder* lastPlaceHolder = (EmptyPlaceHolder*)&blockBegin[m_metaData.bytePositionOfLastEmptyBlock];
        lastPlaceHolder->bytePositionOfNextEmptyBlock = currentBytePos;
    }

    // set global values
    m_metaData.bytePositionOfLastEmptyBlock = currentBytePos;
    if(m_metaData.bytePositionOfFirstEmptyBlock == UNINIT_STATE_32) {
        m_metaData.bytePositionOfFirstEmptyBlock = currentBytePos;
    }

    m_metaData.numberOfDeletedDynamicItems++;

    return true;
}

#ifdef RUN_UNIT_TEST
/**
 * @brief Cluster::setTestRandValue
 * @param fakeRandVal
 */
void
Cluster::setTestRandValue(const uint32_t fakeRandVal)
{
    m_fakeRandVal = fakeRandVal;
}
#endif

}
