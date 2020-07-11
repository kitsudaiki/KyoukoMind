/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef EDGES_H
#define EDGES_H

#include <common.h>

namespace KyoukoMind
{
class Brick;

//==================================================================================================

struct Edge
{
    float weight = 0.0000001f;
    uint32_t targetId = UNINIT_STATE_32;
    uint32_t brickId = UNINIT_STATE_32;
    uint8_t padding[4];

    // total size: 16 Byte
} __attribute__((packed));

//==================================================================================================

struct EdgeSection
{
    Edge edges[256];

    EdgeSection()
    {
        for(uint16_t i = 0; i < 256; i++)
        {
            Edge newEdge;
            edges[i] = newEdge;
        }
    }

    // total size: 4096 Byte
} __attribute__((packed));

//==================================================================================================

} // namespace KyoukoMind

#endif // EDGES_H
