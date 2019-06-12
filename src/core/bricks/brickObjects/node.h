/**
 *  @file    node.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef NODES_H
#define NODES_H

#include <common.h>

namespace KyoukoMind
{

/**
 * @brief The Node struct
 */
struct Node
{
    float currentState = 0.0f;
    float border = 100.0f;

    float potential = 0.0f;
    uint8_t refractionTime = 0;

    uint8_t active = 0;
    uint8_t tooHeight = 0;

    // Axon
    uint64_t targetBrickPath = 0;
    uint32_t targetAxonId = 0;

} __attribute__((packed));

}

#endif // NODES_H
