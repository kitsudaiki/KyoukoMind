#ifndef NETSTRUCTS_H
#define NETSTRUCTS_H

#include <common/enums.h>
#include <common/includes.h>

struct KyoChanEdge
{
    float weight = 0.0;
    uint32_t targetClusterPath = 0;
    uint16_t targetNodeId = 0;
} __attribute__((packed));

struct KyoChanEdgeSection
{
    uint16_t axonIdInCluster = 0;
    uint16_t numberOfActiveEdges = 0;
    KyoChanEdge edges[50];
    uint64_t padding = 0;
} __attribute__((packed));

struct KyoChanNode
{
    float currentState = 0;
    float border = 0;

    // Axon
    uint32_t targetClusterPath = 0;
    uint16_t targetAxonId = 0;
} __attribute__((packed));

struct KyoChanAxon
{
    float currentState = 0;
} __attribute__((packed));


#endif // NETSTRUCTS_H
