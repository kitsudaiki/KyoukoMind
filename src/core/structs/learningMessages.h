#ifndef LEARNINGMESSAGES_H
#define LEARNINGMESSAGES_H

#include <common.h>

struct KyoChanNewEdge
{
    float weight = 0.0;
    uint32_t newEdgeId = 0;
    uint32_t sourceClusterId = 0;
    uint16_t sourceNodeId = 0;
} __attribute__((packed));

struct KyoChanNewEdgeReply
{
    uint32_t newEdgeId = 0;
    uint32_t sourceClusterId = 0;
    uint16_t sourceNodeId = 0;
    uint32_t targetClusterId = 0;
    uint16_t targetNodeId = 0;
} __attribute__((packed));


struct KyoChanMessage
{

};

#endif // LEARNINGMESSAGES_H
