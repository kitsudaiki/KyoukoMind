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
    uint32_t activeEdges = 0;

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
 * @brief deleteEdge
 * @param section
 * @param position
 */
inline void
deleteEdge(EdgeSection &section,
           const uint8_t position)
{
    const uint32_t ok = section.edges[position].targetId != UNINIT_STATE_32;
    section.edges[position].targetId = UNINIT_STATE_32;
    section.activeEdges -= ok * (1 << position);
}

//==================================================================================================

/**
 * @brief addEdge
 * @param section
 * @param position
 * @param newEdge
 */
inline void
addEdge(EdgeSection &section,
        const uint8_t position,
        const Edge newEdge)
{
    section.edges[position] = newEdge;
    section.activeEdges = section.activeEdges | (1 << position);
}

//==================================================================================================

/**
 * @brief updateSynapseWeight
 * @param section
 * @param position
 * @return
 */
inline void
updateEdgeWeight(EdgeSection &section,
                 const uint32_t position,
                 const float weightUpdate)
{
    float diff = section.edges[position].weight;
    section.edges[position].weight += weightUpdate;
    diff -= section.edges[position].weight;
    section.totalWeight -= diff;
}

//==================================================================================================

} // namespace KyoukoMind

#endif // EDGES_H
