/**
 *  @file    outgoing_buffer.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef OUTGOING_BUFFER_H
#define OUTGOING_BUFFER_H

#include <common.h>
#include <core/messaging/message_objects/messages.h>

namespace KyoukoMind
{

struct OutgoingBuffer
{
    uint8_t targetSide = 0;
    uint32_t targetBrickId = 0;

    DataMessage* message = nullptr;

    void initMessage()
    {
        message->targetSide = targetSide;
        message->targetBrickId = targetBrickId;
    }
} __attribute__((packed));

} // namespace KyoukoMind

#endif // OUTGOING_BUFFER_H
