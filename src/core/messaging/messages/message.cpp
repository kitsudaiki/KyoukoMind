/**
 *  @file    message.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "message.h"
#include <files/dataBuffer.h>

namespace KyoukoMind
{

/**
 * @brief Message::Message
 * @param clusterId
 * @param site
 */
Message::Message(const ClusterID targetClusterId,
                 const ClusterID sourceClusterId,
                 const uint8_t targetSite)
{
    m_metaData.targetClusterId = targetClusterId;
    m_metaData.messageId = m_metaData.messageId << 32;
    m_metaData.messageId += sourceClusterId;
    m_metaData.targetSite = targetSite;

    initBuffer();
}

/**
 * @brief Message::Message
 * @param site
 */
Message::Message(const ClusterID targetClusterId,
                 const uint8_t targetSite)
{
    m_metaData.targetClusterId = targetClusterId;
    m_metaData.targetSite = targetSite;

    initBuffer();
}

/**
 * @brief Message::Message
 * @param data
 * @param size
 */
Message::Message(void *data, uint32_t size)
{
    m_buffer = new PerformanceIO::DataBuffer(data, size);
    m_currentBufferPos = size;
    m_currentBufferSize = m_buffer->getBlockSize() * m_buffer->getNumberOfBlocks();

    memcpy((void*)(&m_metaData), m_buffer->getBufferPointer(), sizeof(CommonMessageData));
}

/**
 * @brief Message::~Message
 */
Message::~Message()
{
    if(m_buffer != nullptr) {
        m_buffer->closeBuffer(true);
    }
}

/**
 * @brief Message::closeBuffer
 * @return
 */
bool Message::closeBuffer()
{
    if(m_buffer != nullptr) {
        m_buffer->closeBuffer(true);
        return true;
    }
    return false;
}

/**
 * @brief Message::initBuffer
 */
void Message::initBuffer()
{
    m_buffer = new PerformanceIO::DataBuffer();
    memcpy(m_buffer->getBufferPointer(), (void*)(&m_metaData), sizeof(CommonMessageData));
    m_currentBufferPos = sizeof(CommonMessageData);
    m_currentBufferSize = m_buffer->getBlockSize() * m_buffer->getNumberOfBlocks();
}

/**
 * @brief Message::metaData
 * @return
 */
CommonMessageData Message::getMetaData() const
{
    return m_metaData;
}

/**
 * @brief Message::getType
 * @return
 */
uint8_t Message::getType() const
{
    return m_metaData.type;
}

/**
 * @brief Message::setMetaData
 * @param targetClusterId
 * @param sourceClusterId
 * @param messageIdCounter
 * @param targetSite
 */
void Message::setMetaData(const ClusterID targetClusterId,
                          const ClusterID sourceClusterId,
                          const uint32_t messageIdCounter,
                          const uint8_t targetSite)
{
    m_metaData.targetClusterId = targetClusterId;
    m_metaData.messageId = messageIdCounter;
    m_metaData.messageId = m_metaData.messageId << 32;
    m_metaData.messageId += sourceClusterId;
    m_metaData.targetSite = targetSite;

    memcpy(m_buffer->getBufferPointer(), (void*)(&m_metaData), sizeof(CommonMessageData));
}

/**
 * @brief Message::getDataSize
 * @return
 */
uint32_t Message::getDataSize() const
{
    return m_currentBufferPos;
}

/**
 * @brief Message::getData
 * @return
 */
void *Message::getData() const
{
    return m_buffer->getBufferPointer();
}

/**
 * @brief Message::getPayloadSize
 * @return
 */
uint32_t Message::getPayloadSize() const
{
    return m_currentBufferPos - sizeof(CommonMessageData);
}

/**
 * @brief Message::getNumberOfPayloadObj
 * @return
 */
uint32_t Message::getNumberOfPayloadObj() const
{
    return (m_currentBufferPos - sizeof(CommonMessageData)) / 20;
}

/**
 * @brief Message::getPayload
 * @return
 */
void *Message::getPayload() const
{
    return m_buffer->getBufferPointer() + sizeof(CommonMessageData);
}

}
