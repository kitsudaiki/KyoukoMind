/**
 *  @file    outgoingMessageBuffer.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/messaging/outgoingMessageBuffer.h>
#include <core/messaging/incomingMessageBuffer.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>

namespace KyoukoMind
{

/**
 * @brief OutgoingMessageBuffer::OutgoingMessageBuffer
 * @param clusterId
 * @param controller
 */
OutgoingMessageBuffer::OutgoingMessageBuffer(const uint32_t targetClusterId,
                                             const uint8_t targetSite,
                                             const uint32_t sourceClusterId)
{
    m_targetClusterId = targetClusterId;
    m_targetSite = targetSite;
    m_sourceClusterId = sourceClusterId;

    m_currentMessage = new DataMessage();
}

/**
 * @brief OutgoingMessageBuffer::OutgoingMessageBuffer
 */
OutgoingMessageBuffer::OutgoingMessageBuffer()
{
    m_currentMessage = new DataMessage();
}

/**
 * @brief OutgoingMessageBuffer::getMessageId
 * @return
 */
uint64_t
OutgoingMessageBuffer::getMessageId()
{
    m_messageIdCounter++;
    return (((m_clusterId * 17) + m_side) << 28) + m_messageIdCounter;
}

/**
 * @brief OutgoingMessageBuffer::finishCycle
 * @param targetClusterId
 * @param targetSite
 * @param sourceClusterId
 * @param neighborInfo
 * @return
 */
bool
OutgoingMessageBuffer::finishCycle(const uint32_t targetClusterId,
                                   const uint8_t targetSite,
                                   const uint32_t sourceClusterId,
                                   const NeighborInformation neighborInfo)
{
    m_currentMessage->setCommonInfo(targetClusterId, targetSite, sourceClusterId);
    m_currentMessage->setNeighborInformation(neighborInfo);
    m_currentMessage->updateMessageHeader();

    if(m_incomingBuffer != nullptr) {
        m_incomingBuffer->addMessage(m_currentMessage);

        m_currentMessage = new DataMessage(m_targetClusterId,
                                           m_targetSite,
                                           m_sourceClusterId,
                                           getMessageId());
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::setIncomingBuffer
 * @param incomingBuffer
 */
void
OutgoingMessageBuffer::setIncomingBuffer(IncomingMessageBuffer *incomingBuffer)
{
    m_incomingBuffer = incomingBuffer;
}

}
