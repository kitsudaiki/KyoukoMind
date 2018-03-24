#ifndef CLUSTERSTRUCTS_H
#define CLUSTERSTRUCTS_H

#include <common/enums.h>
#include <common/includes.h>

struct ClusterID
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;

    bool operator<(const ClusterID& rhs) const
    {
        if (x < rhs.x) {
           return true;
        }
        else if (x == rhs.x) {
            if (y < rhs.y) {
                return true;
            }
            else if (y == rhs.y) {
                return z < rhs.z;
            }
        }
        return false;
    }
    bool operator=(const ClusterID& rhs) const
    {
        if (x == rhs.x && y == rhs.y && z == rhs.z) {
           return true;
        }
        return false;
    }
} __attribute__((packed));

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
