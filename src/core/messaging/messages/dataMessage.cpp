/**
 *  @file    dataMessage.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "dataMessage.h"
#include <files/dataBuffer.h>

namespace KyoukoMind
{

/**
 * @brief DataMessage::DataMessage
 * @param clusterId
 * @param site
 */
DataMessage::DataMessage(const ClusterID targetClusterId,
                         const ClusterID sourceClusterId,
                         const uint8_t targetSite) :
    Message(targetClusterId, sourceClusterId, targetSite)
{
    m_metaData.type = DATA_MESSAGE;
    m_metaData.requiredReply = 1;

    memcpy(m_buffer->getBufferPointer(), (void*)(&m_metaData), sizeof(CommonMessageData));
    m_currentBufferPos = sizeof(CommonMessageData);
}

/**
 * @brief DataMessage::DataMessage
 * @param data
 */
DataMessage::DataMessage(void *data, uint32_t size) : Message(data, size)
{}

/**
 * @brief DataMessage::checkBuffer
 */
inline void DataMessage::checkBuffer(const uint8_t size)
{
    if(m_currentBufferPos + size > m_currentBufferSize) {
        m_buffer->allocateBlocks(1);
        m_currentBufferSize += m_buffer->getBlockSize() * m_buffer->getNumberOfBlocks();
    }
}

/**
 * @brief DataMessage::copyToBuffer
 * @param data
 */
inline void DataMessage::copyToBuffer(void *data, const uint8_t size)
{
    memcpy(m_buffer->getBufferPointer() + m_currentBufferPos,
           data,
           size);
    m_currentBufferPos += size;
}

/**
 * @brief DataMessage::addDirectEdge
 * @param newEdge
 */
void DataMessage::addDirectEdge(const KyoChanDirectEdgeContainer *newEdge)
{
    const uint8_t size = sizeof(KyoChanDirectEdgeContainer);
    checkBuffer(size);
    copyToBuffer((void*)newEdge, size);
}

/**
 * @brief DataMessage::addForwardEdge
 * @param newEdge
 */
void DataMessage::addForwardEdge(const KyoChanForwardEdgeContainer *newEdge)
{
    const uint8_t size = sizeof(KyoChanForwardEdgeContainer);
    checkBuffer(size);
    copyToBuffer((void*)newEdge, size);
}

/**
 * @brief DataMessage::addAxonEdge
 * @param newAxonEdge
 */
void DataMessage::addAxonEdge(const KyoChanAxonEdgeContainer* newAxonEdge)
{
    const uint8_t size = sizeof(KyoChanAxonEdgeContainer);
    checkBuffer(size);
    copyToBuffer((void*)newAxonEdge, size);
}

/**
 * @brief DataMessage::addNewEdge
 * @param newEdge
 */
void DataMessage::addNewEdge(const KyoChanLearingEdgeContainer *newEdge)
{
    const uint8_t size = sizeof(KyoChanLearingEdgeContainer);
    checkBuffer(size);
    copyToBuffer((void*)newEdge, size);
}

/**
 * @brief DataMessage::addNewEdgeReply
 * @param newEdgeReply
 */
void DataMessage::addNewEdgeReply(const KyoChanLearningEdgeReplyContainer *newEdgeReply)
{
    const uint8_t size = sizeof(KyoChanLearningEdgeReplyContainer);
    checkBuffer(size);
    copyToBuffer((void*)newEdgeReply, size);
}

}
