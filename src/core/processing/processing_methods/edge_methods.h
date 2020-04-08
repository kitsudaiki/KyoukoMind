#ifndef EDGE_METHODS_H
#define EDGE_METHODS_H

#include <common.h>
#include <core/objects/edges.h>

namespace KyoukoMind
{

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

}
#endif // EDGE_METHODS_H
