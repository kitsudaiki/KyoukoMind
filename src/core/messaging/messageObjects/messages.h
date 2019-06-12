/**
 *  @file    messages.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef MESSAGES_H
#define MESSAGES_H

#include <common.h>

namespace KyoukoMind
{

/**
 * @brief The MessageType enum
 */
enum MessageType
{
    UNDEFINED_MESSAGE = 0,
    DATA_MESSAGE = 1,
    REPLY_MESSAGE = 2,
};

struct DataMessage
{
    uint8_t type = DATA_MESSAGE;
    uint8_t isLast = 0;

    uint32_t targetBrickId = UNINIT_STATE_32;
    uint8_t targetSide = 0;

    uint64_t currentPosition = UNINIT_STATE_64;
    uint64_t prePosition = UNINIT_STATE_64;

    uint16_t size = 0;
    uint8_t data[487];

    void init()
    {
        type = DATA_MESSAGE;
        isLast = 0;

        targetBrickId = UNINIT_STATE_32;
        targetSide = 0;

        currentPosition = UNINIT_STATE_64;
        prePosition = UNINIT_STATE_64;
        size = 0;
    }

    void reset()
    {
        prePosition = UNINIT_STATE_64;
        currentPosition = UNINIT_STATE_64;
        isLast = 0;
        size = 0;
        memset(data, 0, 486);
    }

} __attribute__((packed));

struct NeighborInformation
{
    uint16_t numberOfActiveNodes = 0;
    float localMemorizing = 0.0f;
    float localLearing = 0.0f;
} __attribute__((packed));


struct ReplyMessageHeader
{
    uint8_t type = REPLY_MESSAGE;
    uint64_t requiredReply = 0;
} __attribute__((packed));


}

#endif // MESSAGES_H
