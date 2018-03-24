#ifndef MESSAGESTRUCTS_H
#define MESSAGESTRUCTS_H

#include <common/enums.h>
#include <common/includes.h>

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

#endif // MESSAGESTRUCTS_H
