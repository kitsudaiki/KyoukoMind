/**
 *  @file    messageHeader.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef MESSAGEOBJECTS_H
#define MESSAGEOBJECTS_H

#include <common.h>

/**
 * @brief The MessageType enum
 */
enum MessageType
{
    UNDEFINED_MESSAGE = 0,
    DATA_MESSAGE = 1,
    REPLY_MESSAGE = 2,
};

/**
 * @brief The CommonMessageInformation struct
 */
struct CommonMessageInformation
{
    uint8_t type = UNDEFINED_MESSAGE;
    uint32_t targetClusterId = 0;
    uint8_t targetSide = 0;
    uint32_t sourceClusterId = 0;
    uint8_t requiredReply = 0;
} __attribute__((packed));

/**
 * @brief The NeighborInformation struct
 */
struct NeighborInformation
{
    uint16_t numberOfActiveNodes = 0;
    float localMemorizing = 0.0f;
    float localLearing = 0.0f;

    /**
     * @brief addValues
     * @param info
     */
    void addValues(const NeighborInformation &info)
    {
        numberOfActiveNodes += info.numberOfActiveNodes;
        localMemorizing += info.localMemorizing;
        localLearing += info.localLearing;
    }

    /**
     * @brief reset
     */
    void reset()
    {
        numberOfActiveNodes = 0;
        localMemorizing = 0.0f;
        localLearing = 0.0f;
    }
} __attribute__((packed));

/**
 * @brief The ReplyMessageHeader struct
 */
struct ReplyMessageHeader
{
    uint8_t type = REPLY_MESSAGE;
    CommonMessageInformation commonInfo;
    uint64_t messageId = 0;
} __attribute__((packed));

/**
 * @brief The DataMessageHeader struct
 */
struct DataMessageHeader
{
    CommonMessageInformation commonInfo;
    uint64_t messageId = 0;
    uint32_t payloadSize = 0;
    NeighborInformation neighborInfos;
} __attribute__((packed));

#endif // MESSAGEOBJECTS_H
