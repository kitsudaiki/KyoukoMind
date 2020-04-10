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

} __attribute__((packed));

//==================================================================================================

struct EdgeSection
{
    uint8_t status = ACTIVE_SECTION;
    Edge edges[10];
    uint32_t activeEdges = 0;

    float totalWeight = 0.0f;

    uint8_t sourceSide = 0;
    uint32_t sourceId = UNINIT_STATE_32;

    EdgeSection()
    {
        for(uint32_t i = 0; i < 25; i++)
        {
            Edge newEdge;
            edges[i] = newEdge;
        }
    }
} __attribute__((packed));

} // namespace KyoukoMind

#endif // EDGES_H
