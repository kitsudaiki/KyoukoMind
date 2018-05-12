/**
 *  @file    kyochanNodes.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef KYOCHANNODES_H
#define KYOCHANNODES_H

#include <common.h>
#include <core/structs/kyochanEdges.h>

/**
 * @brief The KyoChanNode struct
 */
struct KyoChanNode
{
    float currentState = 0;
    float border = 10;

    // Axon
    uint64_t targetClusterPath = 0;
    uint32_t targetAxonId = 0;

} __attribute__((packed));

/**
 * @brief The PossibleKyoChanNodes struct
 */
struct PossibleKyoChanNodes
{
    uint16_t nodeIds[MAX_NUMBER_OF_ACTIVE_NODES];
    uint16_t numberOfActiveNodes = 0;

    /**
     * @brief addActiveNodeId
     * @param nodeId
     * @return
     */
    bool addNodeId(const uint16_t &nodeId)
    {
        if(numberOfActiveNodes >= MAX_NUMBER_OF_ACTIVE_NODES) {
            return false;
        }
        nodeIds[numberOfActiveNodes] = nodeId;
        numberOfActiveNodes++;
        return true;
    }
} __attribute__((packed));

#endif // KYOCHANNODES_H
