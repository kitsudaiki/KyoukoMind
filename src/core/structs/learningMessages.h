/**
 *  @file    learingMessages.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef LEARNINGMESSAGES_H
#define LEARNINGMESSAGES_H

#include <common.h>

struct KyoChanLearingEdgeContainer
{
    uint8_t type = LEARNING_CONTAINER;
    float weight = 0.0;
    uint32_t newEdgeId = 0;
    uint32_t sourceClusterPath = 0;
    uint32_t sourceAxonId = 0;
    uint8_t step = 0;
    uint8_t padding[2];
} __attribute__((packed));

struct KyoChanLearningEdgeReplyContainer
{
    uint8_t type = LEARNING_REPLY_CONTAINER;
    uint32_t newEdgeId = 0;
    uint32_t sourceClusterPath = 0;
    uint32_t sourceAxonId = 0;
    uint32_t targetClusterPath = 0;
    uint16_t targetNodeId = 0;
    uint8_t padding[1];
} __attribute__((packed));


#endif // LEARNINGMESSAGES_H
