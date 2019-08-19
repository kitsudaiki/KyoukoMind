/**
 *  @file    content_container.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef CONTENT_CONTAINER_H
#define CONTENT_CONTAINER_H

#include <common.h>

namespace KyoukoMind
{

/**
 * @brief The StatusEdgeContainer struct
 */
struct UpdateEdgeContainer
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
 * @brief The PendingEdgeContainer struct
 */
struct PendingEdgeContainer
{
    uint8_t type = PENDING_EDGE_CONTAINER;

    float weight = 0.0;
    uint32_t sourceEdgeSectionId = 0;
    uint8_t sourceSide = 0;

} __attribute__((packed));

/**
 * @brief The EdgeForewardContainer struct
 */
struct DirectEdgeContainer
{
    uint8_t type = DIRECT_EDGE_CONTAINER;

    float weight = 0.0;
    uint16_t targetNodeId = 0;

} __attribute__((packed));

/**
 * @brief The EdgeForewardContainer struct
 */
struct EdgeContainer
{
    uint8_t type = FOREWARD_EDGE_CONTAINER;

    float weight = 0.0;
    uint32_t targetEdgeSectionId = 0;

} __attribute__((packed));

/**
 * @brief The AxonEdgeContainer struct
 */
struct AxonEdgeContainer
{
    uint8_t type = AXON_EDGE_CONTAINER;

    float weight = 0.0;
    uint32_t targetAxonId = 0;
    uint64_t targetBrickPath = 0;

} __attribute__((packed));

/**
 * @brief The LearingEdgeContainer struct
 */
struct LearingEdgeContainer
{
    uint8_t type = LEARNING_EDGE_CONTAINER;

    uint32_t sourceEdgeSectionId = 0;
    float weight = 0.0;

} __attribute__((packed));

/**
 * @brief The LearningEdgeReplyContainer struct
 */
struct LearningEdgeReplyContainer
{
    uint8_t type = LEARNING_REPLY_EDGE_CONTAINER;

    uint32_t sourceEdgeSectionId = 0;
    uint32_t targetEdgeSectionId = 0;

} __attribute__((packed));

/**
 * @brief The DeleteContainer struct
 */
struct DeleteContainer
{
    uint8_t type = DELETE_CONTAINER;

    uint32_t targetEdgeSectionId = 0;

} __attribute__((packed));

/**
 * @brief The DebugContainer struct
 */
struct DebugContainer
{
    uint8_t type = DEBUG_CONTAINER;

    uint32_t targetEdgeSectionId = 0;
    float weight = 0.0;

} __attribute__((packed));

} // namespace KyoukoMind

#endif // CONTENT_CONTAINER_H
