/**
 *  @file    messageBlock.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef MESSAGEBLOCK_H
#define MESSAGEBLOCK_H

#include <common.h>
#include <core/messaging/messageObjects/messages.h>

namespace KyoukoMind
{

struct MessageBlock
{
DataMessage buffer[MESSAGES_PER_BLOCK];

uint32_t maximumMessages = MESSAGES_PER_BLOCK;
uint32_t containsMessages = 0;
uint32_t processedMessages = 0;

MessageBlock()
{
    reset();
}

~MessageBlock()
{
}

void reset()
{
    containsMessages = 0;
    processedMessages = 0;
    memset(buffer, 0, MESSAGES_PER_BLOCK * sizeof(DataMessage));
}
} __attribute__((packed));

}

#endif // MESSAGEBLOCK_H
