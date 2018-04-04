#ifndef CLUSTERMETA_H
#define CLUSTERMETA_H

#include <common.h>

struct Neighbor
{
    ClusterID targetClusterId;
    uint8_t neighborType = EMPTYCLUSTER;
    uint32_t numberOfConnections = 0;
    uint32_t distantToNextNodeCluster = 0;
} __attribute__((packed));

struct ClusterMetaData
{
    ClusterID clusterId;
    uint8_t clusterType = EMPTYCLUSTER;
    Neighbor neighors[9];
    uint32_t numberOfNeighbors = 1;

    uint32_t numberOfNodes = 0;
    uint32_t numberOfAxons = 0;
    uint32_t numberOfEdgeSections = 0;

    uint32_t positionNodeBlocks = 0;
    uint32_t numberOfNodeBlocks = 0;

    uint32_t positionAxonBlocks = 0;
    uint32_t numberOfAxonBlocks = 0;

    uint32_t positionOfEdgeBlock = 0;
    uint32_t numberOfEdgeBlocks = 0;

} __attribute__((packed));

#endif // CLUSTERMETA_H
