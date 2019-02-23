/**
 *  @file    messageContainer.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef LEARNINGMESSAGES_H
#define LEARNINGMESSAGES_H

#include <common.h>
#include <core/structs/messageHeader.h>

/**
 * @brief The KyoChanStatusEdgeContainer struct
 */
struct KyoChanUpdateEdgeContainer
{
    enum UpdateTypes {
        UNDEF_TYPE = 0,
        SET_TYPE = 1,
        ADD_TYPE = 2,
        SUB_TYPE = 4,
        DELETE_TYPE = 8,
    };

    uint8_t type = STATUS_EDGE_CONTAINER;

    uint32_t targetId = 0;
    float updateValue = 0;
    uint8_t updateType = UNDEF_TYPE;

} __attribute__((packed));

/**
 * @brief The KyoChanPendingEdgeContainer struct
 */
struct KyoChanPendingEdgeContainer
{
    uint8_t type = PENDING_EDGE_CONTAINER;

    float weight = 0.0;
    uint32_t sourceEdgeSectionId = 0;
    uint8_t sourceSide = 0;

} __attribute__((packed));

/**
 * @brief The KyoChanEdgeForewardContainer struct
 */
struct KyoChanDirectEdgeContainer
{
    uint8_t type = DIRECT_EDGE_CONTAINER;

    float weight = 0.0;
    uint16_t targetNodeId = 0;

} __attribute__((packed));

/**
 * @brief The KyoChanEdgeForewardContainer struct
 */
struct KyoChanForwardEdgeContainer
{
    uint8_t type = FOREWARD_EDGE_CONTAINER;

    float weight = 0.0;
    uint32_t targetEdgeSectionId = 0;

} __attribute__((packed));

/**
 * @brief The KyoChanAxonEdgeContainer struct
 */
struct KyoChanAxonEdgeContainer
{
    uint8_t type = AXON_EDGE_CONTAINER;

    float weight = 0.0;
    uint32_t targetAxonId = 0;
    uint64_t targetClusterPath = 0;

} __attribute__((packed));

/**
 * @brief The KyoChanLearingEdgeContainer struct
 */
struct KyoChanLearingEdgeContainer
{
    uint8_t type = LEARNING_EDGE_CONTAINER;

    uint32_t sourceEdgeSectionId = 0;
    float weight = 0.0;

} __attribute__((packed));

/**
 * @brief The KyoChanLearningEdgeReplyContainer struct
 */
struct KyoChanLearningEdgeReplyContainer
{
    uint8_t type = LEARNING_REPLY_EDGE_CONTAINER;

    uint32_t sourceEdgeSectionId = 0;
    uint32_t targetEdgeSectionId = 0;

} __attribute__((packed));

/**
 * @brief The KyoChanDeleteContainer struct
 */
struct KyoChanDeleteContainer
{
    uint8_t type = DELETE_CONTAINER;

    uint32_t targetEdgeSectionId = 0;

} __attribute__((packed));

/**
 * @brief The KyoChanDebugContainer struct
 */
struct KyoChanDebugContainer
{
    uint8_t type = DEBUG_CONTAINER;

    uint32_t targetEdgeSectionId = 0;
    float weight = 0.0;

} __attribute__((packed));


#endif // LEARNINGMESSAGES_H
