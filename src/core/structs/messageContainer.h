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
 * @brief The KyoChanStatusEdgeContainer struct
 */
struct KyoChanStatusEdgeContainer
{
    uint8_t type = STATUS_EDGE_CONTAINER;

    uint32_t targetId = 0;
    float status = 0;

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


#endif // LEARNINGMESSAGES_H
