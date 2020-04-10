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
deleteEdgeByPosition(EdgeSection &section,
                     const uint8_t position)
{
    assert(position < section.activeEdges);
    assert(section.activeEdges > 0);

    section.totalWeight -= section.edges[position].weight;
    section.edges[position] = section.edges[section.activeEdges - 1];
    section.activeEdges--;
}

//==================================================================================================

/**
 * @brief deleteEdge
 * @param section
 * @param position
 */
inline void
deleteEdgeBySide(EdgeSection &section,
                 const uint8_t side)
{
    for(uint8_t pos = 0; pos < section.activeEdges; pos++)
    {
        const uint32_t currentSide = section.edges[pos].side;
        if(currentSide == side)
        {
            deleteEdgeByPosition(section, pos);
            return;
        }
    }
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
        const uint8_t side)
{
    Edge newEdge;
    newEdge.side = side;

    section.edges[section.activeEdges] = newEdge;
    section.activeEdges++;
    section.totalWeight += newEdge.weight;
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
    assert(position < section.activeEdges);

    float diff = section.edges[position].weight;
    section.edges[position].weight += weightUpdate;
    diff -= section.edges[position].weight;
    section.totalWeight -= diff;
}

//==================================================================================================

}
#endif // EDGE_METHODS_H
