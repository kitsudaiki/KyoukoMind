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
    Brick* brick = nullptr;
    uint8_t inputSide = UNINIT_STATE_8;
    uint16_t prev = UNINIT_STATE_16;
    uint8_t numberNext = 0;
    uint16_t next[6];

    Edge()
    {
        for(uint16_t i = 0; i < 6; i++)
        {
            next[i] = UNINIT_STATE_16;
        }
    }
    // total size: 16 Byte
} __attribute__((packed));

//==================================================================================================

struct EdgeSection
{
    uint8_t status = ACTIVE_SECTION;
    uint8_t padding[7];

    uint32_t targetBrickId = UNINIT_STATE_32;
    uint32_t targetBrickDistance = 0;

    uint64_t padding1 = 0;
    uint64_t padding2 = 0;

    Edge edges[127];

    EdgeSection()
    {
        for(uint16_t i = 0; i < 127; i++)
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
