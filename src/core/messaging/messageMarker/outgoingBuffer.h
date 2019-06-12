/**
 *  @file    outgoingBuffer.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef OUTGOINGBUFFER_H
#define OUTGOINGBUFFER_H

#include <common.h>
#include <core/messaging/messageObjects/messages.h>

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

}

#endif // OUTGOINGBUFFER_H
