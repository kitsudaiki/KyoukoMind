/**
 *  @file    message_block.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef MESSAGE_BLOCK_H
#define MESSAGE_BLOCK_H

#include <common.h>
#include <core/messaging/message_objects/messages.h>

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

~MessageBlock() {}

void reset()
{
    containsMessages = 0;
    processedMessages = 0;
    memset(buffer, 0, MESSAGES_PER_BLOCK * sizeof(DataMessage));
}
} __attribute__((packed));

} // namespace KyoukoMind

#endif // MESSAGE_BLOCK_H
