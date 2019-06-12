/**
 *  @file    messageBuffer.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef MESSAGEBLOCKBUFFER_H
#define MESSAGEBLOCKBUFFER_H

#include <common.h>
#include <core/messaging/messageObjects/messages.h>
#include <core/messaging/messageObjects/messageBlock.h>

namespace KyoukoMind
{

class MessageBlockBuffer
{
public:
    MessageBlockBuffer();
    ~MessageBlockBuffer();

    friend class MessageBufferTest;

    uint64_t appandMessageBlock(DataMessage *message);

    void addMessageBlock(const uint64_t pos,
                         const uint8_t* data,
                         const uint8_t messageCount);
    DataMessage* getMessage(const uint64_t pos);
    DataMessage* getNextMessage();
    void finishMessage(const uint64_t pos);

    DataMessage *reserveBuffer(const uint64_t prePosition=UNINIT_STATE_64);
    void finishReservedBuffer(const uint64_t pos);

private:
    std::vector<MessageBlock*> m_blocks;
    MessageBlock* m_reserveBlock = nullptr;
    std::atomic_flag m_lock = ATOMIC_FLAG_INIT;

    uint32_t m_offset = 0;

    uint64_t m_currentGetPos = 0;
    uint64_t m_currentWritePos = 0;

    void checkAndDelete();
};

}

#endif // MESSAGEBLOCKBUFFER_H
