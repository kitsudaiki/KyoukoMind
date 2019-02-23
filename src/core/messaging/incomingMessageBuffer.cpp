/**
 *  @file    incomingMessageBuffer.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/messaging/incomingMessageBuffer.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>

namespace KyoukoMind
{

/**
 * @brief IncomingMessageBuffer::IncomingMessageBuffer
 */
IncomingMessageBuffer::IncomingMessageBuffer()
{
    for(uint8_t i = 0; i < INCOM_BUFFER_SIZE; i++) {
        m_waitingMessages[i] = nullptr;
    }
    m_currentProcessingMessage = new DataMessage();
}

/**
 * @brief IncomingMessageBuffer::~IncomingMessageBuffer
 */
IncomingMessageBuffer::~IncomingMessageBuffer()
{
    clearCurrentMessage();
    // TODO: clear message-buffer
}

/**
 * @brief IncomingMessageBuffer::addMessage
 * @param message
 * @return
 */
bool
IncomingMessageBuffer::addMessage(DataMessage *message)
{
    bool result = false;
    m_mutex.lock();
    if((m_newestBufferPos + 1) % INCOM_BUFFER_SIZE != m_oldestBufferPos)
    {
        m_newestBufferPos = (m_newestBufferPos + 1) % INCOM_BUFFER_SIZE;
        m_waitingMessages[m_newestBufferPos] = message;
        result = true;
    }
    m_mutex.unlock();
    return result;
}

/**
 * @brief IncomingMessageBuffer::getMessage
 * @return
 */
DataMessage*
IncomingMessageBuffer::getMessage()
{
    if(m_currentProcessingMessage != nullptr) {
        return m_currentProcessingMessage;
    }

    m_mutex.lock();
    m_oldestBufferPos = (m_oldestBufferPos + 1) % INCOM_BUFFER_SIZE;
    m_currentProcessingMessage = m_waitingMessages[m_oldestBufferPos];
    m_waitingMessages[m_oldestBufferPos] = nullptr;
    m_mutex.unlock();

    return m_currentProcessingMessage;
}

/**
 * @brief IncomingMessageBuffer::finish
 * @return
 */
bool
IncomingMessageBuffer::finish()
{
    return clearCurrentMessage();
}

/**
 * @brief IncomingMessageBuffer::isReady
 * @return
 */
bool
IncomingMessageBuffer::isReady()
{
    bool result = false;
    m_mutex.lock();
    if((m_currentProcessingMessage == nullptr
            && m_waitingMessages[(m_oldestBufferPos + 1) % INCOM_BUFFER_SIZE] != nullptr)
            || m_currentProcessingMessage != nullptr) {
        result = true;
    }
    m_mutex.unlock();
    return result;
}

/**
 * @brief IncomingMessageBuffer::clearCurrentMessage
 * @return
 */
bool
IncomingMessageBuffer::clearCurrentMessage()
{
    if(m_currentProcessingMessage == nullptr) {
        return false;
    }

    // TODO: watch this, because even the nullpointer-check exist, the closeBuffer failes sometimes
    //       because m_currentProcessingMessage is a null-pointer
    m_currentProcessingMessage->closeBuffer();
    delete m_currentProcessingMessage;
    m_currentProcessingMessage = nullptr;
    return true;
}

}
