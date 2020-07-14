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
    float synapseWeight = 0.0f;
    float edgeWeight1 = 0.0f;
    float edgeWeight2 = 0.0f;
    uint32_t synapseSectionId = UNINIT_STATE_32;
    uint32_t currentBrickId = UNINIT_STATE_32;
    // total size: 20 Byte
} __attribute__((packed));

//==================================================================================================

struct EdgeSection
{
    uint32_t targetBrickId = UNINIT_STATE_32;
    uint8_t padding1[4];

    Edge edges[204];

    uint8_t padding2[8];

    EdgeSection()
    {
        for(uint16_t i = 0; i < 204; i++)
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
