/**
 *  @file    incomingBuffer.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef INCOMINGBUFFER_H
#define INCOMINGBUFFER_H

#include <common.h>

namespace KyoukoMind
{

struct IncomingBuffer
{
    uint64_t bufferPositions[INCOMING_BUFFER_SIZE];
    uint64_t currentProcessingMessage = UNINIT_STATE_64;
    std::atomic_flag lock = ATOMIC_FLAG_INIT;

    uint8_t newestBufferPos = 0;
    uint8_t oldestBufferPos = 0;

    IncomingBuffer()
    {
        for(uint8_t i = 0; i < 5; i++) {
            bufferPositions[i] = UNINIT_STATE_64;
        }
    }

    /**
     * @brief addMessage
     * @param message
     * @return
     */
    bool
    addMessage(const uint64_t position)
    {
        bool result = false;
        while(lock.test_and_set(std::memory_order_acquire)) {
            ; // spin
        }
        if((newestBufferPos + 1) % INCOMING_BUFFER_SIZE != oldestBufferPos)
        {
            newestBufferPos = (newestBufferPos + 1) % INCOMING_BUFFER_SIZE;
            bufferPositions[newestBufferPos] = position;
            result = true;
        }
        lock.clear(std::memory_order_release);
        return result;
    }

    /**
     * @brief getMessage
     * @return
     */
    uint64_t
    getMessage()
    {
        uint64_t returnMessage = UNINIT_STATE_64;
        while(lock.test_and_set(std::memory_order_acquire)) {
            ; // spin
        }
        if(currentProcessingMessage != UNINIT_STATE_64)
        {
            returnMessage = currentProcessingMessage;
            lock.clear(std::memory_order_release);
            return returnMessage;
        }

        oldestBufferPos = (oldestBufferPos + 1) % INCOMING_BUFFER_SIZE;
        currentProcessingMessage = bufferPositions[oldestBufferPos];
        bufferPositions[oldestBufferPos] = UNINIT_STATE_64;

        returnMessage = currentProcessingMessage;

        lock.clear(std::memory_order_release);

        return returnMessage;
    }

    /**
     * @brief finish
     * @return
     */
    void
    reset()
    {
        while(lock.test_and_set(std::memory_order_acquire)) {
            ; // spin
        }

        currentProcessingMessage = UNINIT_STATE_64;
        lock.clear(std::memory_order_release);
    }

    /**
     * @brief isReady
     * @return
     */
    bool
    isReady()
    {
        bool result = false;
        while(lock.test_and_set(std::memory_order_acquire)) {
            ; // spin
        }
        if((currentProcessingMessage == UNINIT_STATE_64
                && bufferPositions[(oldestBufferPos + 1) % INCOMING_BUFFER_SIZE] != UNINIT_STATE_64)
                || currentProcessingMessage != UNINIT_STATE_64)
        {
            result = true;
        }
        lock.clear(std::memory_order_release);
        return result;
    }


} __attribute__((packed));

}

#endif // INCOMINGBUFFER_H
