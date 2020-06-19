/**
 *  @file    node.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#ifndef NODES_H
#define NODES_H

#include <common.h>

namespace KyoukoMind
{

//==================================================================================================

struct Node
{
    float currentState = 0.0f;
    float border = 100.0f;

    float potential = 0.0f;
    uint8_t refractionTime = 0;

    uint8_t active = 0;
    uint8_t tooHigh = 0;

    // Axon
    uint64_t targetBrickPath = 0;
    uint32_t targetAxonId = 0;

    uint8_t padding[5];

} __attribute__((packed));

//==================================================================================================

} // namespace KyoukoMind

#endif // NODES_H
