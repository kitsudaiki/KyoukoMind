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
    Edge edges[25];
    float totalWeight = 0.0000001f;
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

//==================================================================================================

/**
 * summarize all sides of the edge-section
 *
 * @return the total weight of the section
 */
inline float
getTotalWeight(EdgeSection &section)
{
    float result = 0.0000001f;
    for(uint32_t i = 0; i < 25; i++)
    {
        assert(section.edges[i].weight >= 0.0f);
        result += section.edges[i].weight;
    }
    return result;
}

//==================================================================================================

/**
 * count the active sides of the section
 *
 * @return number of active edges in the section
 */
inline uint8_t
getActiveEdges(EdgeSection &section)
{
    uint8_t count = 0;
    for(int i = 0; i < 25; i++)
    {
        if(section.edges[i].targetId != UNINIT_STATE_32
                || section.edges[i].weight > 0.0f)
        {
            count++;
        }
    }
    return count;
}

//==================================================================================================

} // namespace KyoukoMind

#endif // EDGES_H
