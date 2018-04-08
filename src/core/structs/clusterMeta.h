/**
 *  @file    clusterMeta.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef CLUSTERMETA_H
#define CLUSTERMETA_H

#include <common.h>

struct ClusterPos
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;
} __attribute__((packed));

struct Neighbor
{
    ClusterID targetClusterId;
    ClusterPos targetClusterPos;

    uint8_t side = 0;
    uint8_t neighborType = EMPTYCLUSTER;
    uint32_t numberOfConnections = 0;
    uint32_t distantToNextNodeCluster = 0;
} __attribute__((packed));

struct ClusterMetaData
{
    ClusterID clusterId;
    ClusterPos clusterPos;

    uint8_t clusterType = EMPTYCLUSTER;
    Neighbor neighors[10];
    uint32_t numberOfNeighbors = 1;

    uint16_t numberOfNodes = 0;
    uint32_t numberOfAxons = 0;
    uint32_t numberOfEdgeSections = 0;

    uint32_t positionNodeBlocks = 0;
    uint32_t numberOfNodeBlocks = 0;

    uint32_t positionAxonBlocks = 0;
    uint32_t numberOfAxonBlocks = 0;

    uint32_t positionOfEdgeBlock = 0;
    uint32_t numberOfEdgeBlocks = 0;

} __attribute__((packed));

namespace KyoukoMind {
class Cluster;
}

struct MetaDataEntry {
    uint8_t type = 0;
    ClusterID clusterId = 0;
    Neighbor neighbors[10];
    KyoukoMind::Cluster* cluster = nullptr;
    uint32_t numberOfAxons = 0;
};

#endif // CLUSTERMETA_H
