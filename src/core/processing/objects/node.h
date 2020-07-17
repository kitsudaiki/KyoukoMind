/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
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
    uint8_t padding[2];

    uint32_t brickId = 0;
    uint32_t targetBrickDistance = 0;

    // total size: 24 Byte
};

//==================================================================================================

} // namespace KyoukoMind

#endif // NODES_H
