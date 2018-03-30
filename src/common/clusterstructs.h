#ifndef CLUSTERSTRUCTS_H
#define CLUSTERSTRUCTS_H

#include <common/enums.h>
#include <common/includes.h>
#include <common/typedefs.h>

struct Neighbor
{
    ClusterID targetClusterId;
    ClusterType neighborType = EMPTYCLUSTER;
    uint32_t numberOfConnections = 0;
    uint32_t distantToNextNodeCluster = 0;
} __attribute__((packed));

struct ClusterMetaData
{
    ClusterID clusterId;
    ClusterType clusterType = EMPTYCLUSTER;
    Neighbor neighors[9];
    uint32_t numberOfNeighbors = 1;
    uint32_t numberOfNodes = 0;
    uint32_t positionNodeBlock = 0;
    uint32_t numberOfNodeBlocks = 0;
    uint32_t positionOfEdgeBlock = 0;
    uint32_t numberOfEdgeBlocks = 0;

} __attribute__((packed));

#endif // CLUSTERSTRUCTS_H
