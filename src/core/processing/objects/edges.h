/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef EDGES_H
#define EDGES_H

#include <common.h>

namespace KyoukoMind
{

//==================================================================================================

struct Edge
{
    float weight = 0.0000001f;
    int32_t targetId = 0;
    uint8_t used = 0;
    uint8_t synapseConnection = 0;
    uint8_t next[6];
    // total size: 16 Byte
} __attribute__((packed));

//==================================================================================================

struct EdgeSection
{
    uint8_t status = ACTIVE_SECTION;
    uint8_t padding[5];

    uint16_t firstEdge = 0;

    uint32_t targetBrickId = UNINIT_STATE_32;
    uint32_t targetBrickDistance = 0;

    Edge edges[255];

    EdgeSection()
    {
        for(uint16_t i = 0; i < 255; i++)
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
