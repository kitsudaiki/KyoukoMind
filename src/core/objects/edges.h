/**
 *  @file    edges.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef EDGES_H
#define EDGES_H

#include <common.h>

namespace KyoukoMind
{

//==================================================================================================

struct Edge
{
    float weight = 0.0;
    uint32_t targetId = UNINIT_STATE_32;
    uint8_t available = 0;

} __attribute__((packed));

//==================================================================================================

struct EdgeSection
{
    uint8_t status = ACTIVE_SECTION;

    Edge edges[23];
    float totalWeight = 0.0000001f;

    uint8_t sourceSide = UNINIT_STATE_8;
    uint32_t sourceId = UNINIT_STATE_32;

    EdgeSection()
    {
        for(uint8_t side = 0; side < 23; side++)
        {
            Edge newEdge;
            edges[side] = newEdge;
        }
    }

} __attribute__((packed));

//==================================================================================================

} // namespace KyoukoMind

#endif // EDGES_H
