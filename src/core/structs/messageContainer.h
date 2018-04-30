/**
 *  @file    messageContainer.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef LEARNINGMESSAGES_H
#define LEARNINGMESSAGES_H

#include <common.h>

/**
 * @brief The KyoChanEdgeForewardContainer struct
 */
struct KyoChanEdgeForewardContainer
{
    uint8_t type = EDGE_FOREWARD_CONTAINER;
    uint8_t size = 10;

    float weight = 0.0;
    uint32_t targetEdgeSection = 0;

} __attribute__((packed));

/**
 * @brief The KyoChanAxonEdgeContainer struct
 */
struct KyoChanAxonEdgeContainer
{
    uint8_t type = AXON_EDGE_CONTAINER;
    uint8_t size = 16;

    float weight = 0.0;
    uint32_t targetAxonId = 0;
    uint64_t targetClusterPath = 0;

} __attribute__((packed));

/**
 * @brief The KyoChanPendingEdgeContainer struct
 */
struct KyoChanPendingEdgeContainer
{
    uint8_t type = PENDING_EDGE_CONTAINER;
    uint8_t size = 8;

    uint16_t marker = 0;
    float weight = 0.0;

} __attribute__((packed));

/**
 * @brief The KyoChanLearingEdgeContainer struct
 */
struct KyoChanLearingEdgeContainer
{
    uint8_t type = LEARNING_CONTAINER;
    uint8_t size = 12;

    uint16_t marker = 0;
    uint32_t sourceEdgeSectionId = 0;
    float weight = 0.0;

} __attribute__((packed));

/**
 * @brief The KyoChanLearningEdgeReplyContainer struct
 */
struct KyoChanLearningEdgeReplyContainer
{
    uint8_t type = LEARNING_REPLY_CONTAINER;
    uint8_t size = 10;

    uint32_t sourceEdgeSectionId = 0;
    uint32_t targetEdgeSectionId = 0;

} __attribute__((packed));


#endif // LEARNINGMESSAGES_H
