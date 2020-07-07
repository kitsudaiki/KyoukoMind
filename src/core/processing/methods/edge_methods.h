#ifndef EDGE_METHODS_H
#define EDGE_METHODS_H

#include <common.h>
#include <core/processing/objects/edges.h>

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
           const uint8_t side)
{

    section.edges[side].targetId = UNINIT_STATE_32;
    section.totalWeight -= section.edges[side].weight;
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
                 const uint32_t side,
                 const float weightUpdate)
{
    float diff = section.edges[side].weight;
    section.edges[side].weight += weightUpdate;
    diff -= section.edges[side].weight;
    section.totalWeight -= diff;
}

//==================================================================================================

}

#endif // EDGE_METHODS_H
