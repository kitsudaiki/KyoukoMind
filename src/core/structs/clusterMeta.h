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

/**
 * @brief The ClusterPos struct
 */
struct ClusterPos
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;
} __attribute__((packed));

/**
 * @brief The Neighbor struct
 */
struct Neighbor
{
    ClusterID targetClusterId = 0;
    ClusterPos targetClusterPos;

    uint8_t targetSide = 0;
    uint8_t neighborType = EMPTY_CLUSTER;
    uint32_t distantToNextNodeCluster = 0;
    uint16_t activeNodesInNextNodeCluster = 0;
} __attribute__((packed));

/**
 * @brief The ClusterMetaData struct
 */
struct ClusterMetaData
{
    ClusterID clusterId = 0;
    ClusterPos clusterPos;

    uint8_t clusterType = EMPTY_CLUSTER;
    Neighbor neighors[17];

    uint8_t outgoing = 0;
    uint8_t incoming = 0;

    uint16_t numberOfNodes = 0;
    uint32_t numberOfForwardEdgeSections = 0;
    uint32_t numberOfEdgeSections = 0;
    uint32_t numberOfPendingForwardEdgeSections = 0;

    uint32_t positionNodeBlocks = 1;
    uint32_t numberOfNodeBlocks = 0;

    uint32_t positionForwardEdgeBlocks = 1;
    uint32_t numberOfForwardEdgeBlocks = 0;

    uint32_t positionOfEdgeBlock = 1;
    uint32_t numberOfEdgeBlocks = 0;

} __attribute__((packed));

namespace KyoukoMind {
class EdgeCluster;
}

/**
 * @brief The InitMetaDataEntry struct
 */
struct InitMetaDataEntry
{
    uint8_t type = 0;
    ClusterID clusterId = 0;
    Neighbor neighbors[17];
    KyoukoMind::EdgeCluster* cluster = nullptr;
    uint32_t numberOfAxons = 0;
};

#endif // CLUSTERMETA_H
