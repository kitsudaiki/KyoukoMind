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

namespace Networking
{
class IncomingMessageBuffer;
class OutgoingMessageBuffer;
}
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
    ClusterID targetClusterId = UNINIT_STATE;
    ClusterPos targetClusterPos;
    uint8_t targetSide = 0;

    Networking::IncomingMessageBuffer* incomBuffer = nullptr;
    Networking::OutgoingMessageBuffer* outgoBuffer = nullptr;
} __attribute__((packed));

/**
 * @brief The ClusterMetaData struct
 */
struct ClusterMetaData
{
    ClusterID clusterId = UNINIT_STATE;
    ClusterPos clusterPos;

    uint8_t clusterType = EMPTY_CLUSTER;
    Neighbor neighors[17];

    uint16_t numberOfNodes = 0;
    uint32_t numberOfForwardEdgeSections = 0;
    uint32_t numberOfEdgeSections = 0;
    uint32_t numberOfPendingForwardEdgeSections = 0;

    uint32_t positionNodeBlocks = 0;
    uint32_t numberOfNodeBlocks = 0;

    uint32_t positionForwardEdgeBlocks = 0;
    uint32_t numberOfForwardEdgeBlocks = 0;

    uint32_t positionOfEdgeBlock = 0;
    uint32_t numberOfEdgeBlocks = 0;

} __attribute__((packed));

namespace KyoukoMind {
class Cluster;
}

/**
 * @brief The InitMetaDataEntry struct
 */
struct InitMetaDataEntry
{
    uint8_t type = 0;
    ClusterID clusterId = 0;
    Neighbor neighbors[17];
    KyoukoMind::Cluster* cluster = nullptr;
    uint32_t numberOfAxons = 0;
    KyoukoMind::Cluster* nodeCluster = nullptr;
};

#endif // CLUSTERMETA_H
