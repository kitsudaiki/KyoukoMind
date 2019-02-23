/**
 *  @file    message.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/messaging/messages/message.h>
#include <commonDataBuffer.h>

namespace KyoukoMind
{

/**
 * @brief Message::Message
 * @param clusterId
 * @param site
 */
Message::Message(const uint32_t targetClusterId,
                 const uint8_t targetSite,
                 const uint32_t sourceClusterId)
{
    m_commonMessageInfo.targetClusterId = targetClusterId;
    m_commonMessageInfo.targetSide = targetSite;
    m_commonMessageInfo.sourceClusterId = sourceClusterId;

    initBuffer();
}

/**
 * @brief Message::Message
 */
Message::Message()
{
    initBuffer();
}

/**
 * @brief Message::~Message
 */
Message::~Message()
{
    closeBuffer();
}

/**
 * @brief Message::setCommonInfo
 * @param targetClusterId
 * @param targetSite
 * @param sourceClusterId
 */
void
Message::setCommonInfo(const uint32_t targetClusterId,
                            const uint8_t targetSite,
                            const uint32_t sourceClusterId)
{
    m_commonMessageInfo.targetClusterId = targetClusterId;
    m_commonMessageInfo.targetSide = targetSite;
    m_commonMessageInfo.sourceClusterId = sourceClusterId;
}

/**
 * @brief Message::closeBuffer
 * @return
 */
bool
Message::closeBuffer()
{
    if(m_buffer != nullptr) {
        delete m_buffer;
        m_buffer = nullptr;
        return true;
    }
    return false;
}

/**
 * @brief Message::initBuffer
 */
void
Message::initBuffer()
{
    m_buffer = new Kitsune::CommonDataBuffer();
}

/**
 * @brief Message::getType
 * @return
 */
uint8_t
Message::getType() const
{
    uint8_t* tempPointer = m_buffer->getBufferPointer();
    return tempPointer[0];
}

/**
 * @brief Message::getData
 * @return
 */
uint8_t*
Message::getData() const
{
    return m_buffer->getBufferPointer();
}

/**
 * @brief Message::getTotalDataSize
 * @return
 */
uint64_t
Message::getTotalDataSize() const
{
    return m_buffer->getNumberOfWrittenBytes();
}

}
