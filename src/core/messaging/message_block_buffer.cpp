/**
 *  @file    message_block_buffer.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "message_block_buffer.h"
#include <kyouko_network.h>
#include <core/bricks/brick_objects/brick.h>

namespace KyoukoMind
{

MessageBlockBuffer::MessageBlockBuffer()
{

}

MessageBlockBuffer::~MessageBlockBuffer()
{
    while(m_lock.test_and_set(std::memory_order_acquire)) {
        ; // spin
    }

    // delete all blocks
    for(uint32_t i = 0; i < m_blocks.size(); i++)
    {
        delete m_blocks[i];
    }

    // delete reserve
    if(m_reserveBlock != nullptr) {
        delete m_reserveBlock;
    }

    m_lock.clear(std::memory_order_release);
}

/**
 * @brief MessageBuffer::reserveBuffer
 * @return
 */
DataMessage*
MessageBlockBuffer::reserveBuffer(const uint64_t prePosition)
{
    DataMessage* result = nullptr;

    while(m_lock.test_and_set(std::memory_order_acquire)) {
        ; // spin
    }

    const uint64_t arrayPos = (m_currentWritePos / MESSAGES_PER_BLOCK) - m_offset;
    const uint32_t positionInBlock = (m_currentWritePos % MESSAGES_PER_BLOCK);

    while(m_blocks.size() <= arrayPos)
    {
        if(m_reserveBlock == nullptr)
        {
            m_blocks.push_back(new MessageBlock());
        }
        else
        {
            m_blocks.push_back(m_reserveBlock);
            m_reserveBlock = nullptr;
        }
    }

    m_blocks[arrayPos]->containsMessages++;
    result = &m_blocks[arrayPos]->buffer[positionInBlock];
    result->init();
    result->currentPosition = m_currentWritePos;
    result->prePosition = prePosition;
    m_currentWritePos++;

    m_lock.clear(std::memory_order_release);

    return result;
}

/**
 * @brief MessageBuffer::finishReservedBuffer
 */
void
MessageBlockBuffer::finishReservedBuffer(const uint64_t pos)
{
    DataMessage* bufferPointer = getMessage(pos);
    assert(bufferPointer->targetBrickId != UNINIT_STATE_32);

    if(bufferPointer->isLast == 1)
    {
        Brick* brick = KyoukoNetwork::m_brickHandler->getBrick(bufferPointer->targetBrickId);
        brick->neighbors[bufferPointer->targetSide].incomBuffer.addMessage(pos);

        if(brick->isReady()) {
            KyoukoNetwork::m_brickHandler->addToQueue(brick);
        }
    }
}

/**
 * @brief IncomingMessageBuffer::appandMessageBlock
 * @param data
 * @return
 */
uint64_t
MessageBlockBuffer::appandMessageBlock(DataMessage *message)
{
    DataMessage* bufferPointer = reserveBuffer();
    const uint64_t tempPosition = bufferPointer->currentPosition;
    memcpy((uint8_t*)bufferPointer, (uint8_t*)message, MESSAGE_SIZE);
    bufferPointer->currentPosition = tempPosition;
    finishReservedBuffer(bufferPointer->currentPosition);
    return bufferPointer->currentPosition;
}

/**
 * @brief MessageBuffer::addMessageBlock
 * @param pos
 * @param data
 */
void
MessageBlockBuffer::addMessageBlock(const uint64_t pos,
                               const uint8_t *data,
                               const uint8_t messageCount)
{
    for(uint32_t i = 0; i < messageCount; i++)
    {
        DataMessage* bufferPointer = reserveBuffer();
        const uint64_t tempPosition = bufferPointer->currentPosition;
        memcpy((uint8_t*)bufferPointer, (uint8_t*)&data[i], MESSAGE_SIZE);
        bufferPointer->currentPosition = tempPosition;
        finishReservedBuffer(bufferPointer->currentPosition);
    }
}

/**
 * @brief MessageBuffer::getMessage
 * @param pos
 * @return
 */
DataMessage*
MessageBlockBuffer::getMessage(const uint64_t pos)
{
    DataMessage* result = nullptr;
    const uint64_t arrayPos = (pos / MESSAGES_PER_BLOCK) - m_offset;
    const uint32_t positionInBlock = (pos % MESSAGES_PER_BLOCK);

    result = &m_blocks[arrayPos]->buffer[positionInBlock];
    return result;
}

/**
 * @brief MessageBuffer::getNextMessage
 * @return
 */
DataMessage*
MessageBlockBuffer::getNextMessage()
{
    DataMessage* result = nullptr;
    while(m_lock.test_and_set(std::memory_order_acquire)) {
        ; // spin
    }

    while(m_currentWritePos > m_currentGetPos)
    {
        const uint64_t arrayPos = (m_currentGetPos / MESSAGES_PER_BLOCK) - m_offset;
        const uint32_t positionInBlock = (m_currentGetPos % MESSAGES_PER_BLOCK);

        m_currentGetPos++;
        if(m_blocks[arrayPos]->buffer[positionInBlock].isLast == 1)
        {
            result = &m_blocks[arrayPos]->buffer[positionInBlock];
            m_lock.clear(std::memory_order_release);
            return result;
        }
    }

    m_lock.clear(std::memory_order_release);

    return result;
}

/**
 * @brief MessageBuffer::finishMessage
 * @param pos
 */
void
MessageBlockBuffer::finishMessage(const uint64_t pos)
{
    while(m_lock.test_and_set(std::memory_order_acquire)) {
        ; // spin
    }

    const uint64_t arrayPos = (pos / MESSAGES_PER_BLOCK) - m_offset;
    const uint32_t positionInBlock = (pos % MESSAGES_PER_BLOCK);

    //assert(m_blocks[arrayPos]->buffer[positionInBlock].type != UNDEFINED_MESSAGE);
    if(m_blocks[arrayPos]->buffer[positionInBlock].type != UNDEFINED_MESSAGE)
    {
        m_blocks[arrayPos]->buffer[positionInBlock].type = UNDEFINED_MESSAGE;
        m_blocks[arrayPos]->processedMessages++;

        checkAndDelete();
    }

    m_lock.clear(std::memory_order_release);
}

/**
 * @brief MessageBuffer::checkAndDelete
 */
void
MessageBlockBuffer::checkAndDelete()
{
    if(m_blocks.size() > 0
            && m_blocks[0]->processedMessages == m_blocks[0]->containsMessages)
    {
        MessageBlock* block = m_blocks[0];
        if(m_reserveBlock == nullptr)
        {
            block->reset();
            m_reserveBlock = block;
        }
        else
        {
            delete block;
        }

        m_blocks.erase(m_blocks.begin(), m_blocks.begin()+1);
        m_offset++;
    }

}

} // namespace KyoukoMind
