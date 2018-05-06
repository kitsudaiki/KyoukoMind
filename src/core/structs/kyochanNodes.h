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
 * @brief The KyoChanAxon struct
 */
struct KyoChanAxon
{
    float currentState = 0;

    uint32_t edgeSectionId = 0;

} __attribute__((packed));

#endif // KYOCHANNODES_H
