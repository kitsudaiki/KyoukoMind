/**
 *  @file    kyochanNodes.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
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
    float currentState = 0.0f;
    float border = 100.0f;

    float potential = 0.0f;
    uint8_t refractionTime = 0;

    uint8_t active = 0;
    uint8_t tooHeight = 0;

    // Axon
    uint64_t targetClusterPath = 0;
    uint32_t targetAxonId = 0;

} __attribute__((packed));

#endif // KYOCHANNODES_H
