/**
 *  @file    clusterMeta.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef CLUSTERMETA_H
#define CLUSTERMETA_H

#include <common.h>

namespace KyoukoMind {
class Cluster;
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
    ClusterID targetClusterId = UNINIT_STATE_32;
    ClusterPos targetClusterPos;
    uint8_t targetSide = 0;

    KyoukoMind::IncomingMessageBuffer* incomBuffer = nullptr;
    KyoukoMind::OutgoingMessageBuffer* outgoBuffer = nullptr;
} __attribute__((packed));

/**
 * @brief The ClusterMetaData struct
 */
struct ClusterMetaData
{
    ClusterID clusterId = UNINIT_STATE_32;
    ClusterPos clusterPos;

    uint8_t inputCluster = 0;
    uint8_t outputCluster = 0;

    uint8_t clusterType = EMPTY_CLUSTER;
    Neighbor neighors[17];

    uint32_t numberOfStaticItems = 0;
    uint32_t numberOfDynamicItems = 0;
    uint32_t numberOfDeletedDynamicItems = 0;

    uint32_t positionOfStaticBlocks = 0;
    uint32_t numberOfStaticBlocks = 0;

    uint32_t positionOfDynamicBlocks = 0;
    uint32_t numberOfDynamicBlocks = 0;

    uint32_t bytePositionOfFirstEmptyBlock = UNINIT_STATE_32;
    uint32_t bytePositionOfLastEmptyBlock = UNINIT_STATE_32;
    uint32_t numberOfEmptyBlocks = 0;

} __attribute__((packed));

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

/**
 * @brief The EmptyPlaceHolder struct
 */
struct EmptyPlaceHolder
{
    uint8_t status = DELETED_SECTION;
    uint32_t bytePositionOfNextEmptyBlock = UNINIT_STATE_32;
} __attribute__((packed));

#endif // CLUSTERMETA_H
