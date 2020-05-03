#ifndef BRICK_ITEM_METHODS_H
#define BRICK_ITEM_METHODS_H

#include <common.h>

#include <core/objects/brick.h>
#include <core/methods/edge_methods.h>
#include <core/methods/data_connection_methods.h>

namespace KyoukoMind
{

//==================================================================================================

inline EdgeSection*
getEdgeBlock(Brick &brick)
{
    return static_cast<EdgeSection*>(brick.edges.buffer.data);
}

//==================================================================================================

/**
 * add a new edge-section to a specific brick with information about the source of the edge
 *
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
inline uint64_t
addEmptyEdgeSection(Brick &brick,
                    const uint8_t sourceSide,
                    const uint32_t sourceId)
{
    assert(sourceSide != 0);
    assert(sourceSide != UNINIT_STATE_8);
    assert(sourceId != UNINIT_STATE_32);

    const uint64_t position = reserveDynamicItem(brick.edges);
    assert(position != UNINIT_STATE_32);

    // create new edge-section
    EdgeSection newSection;
    newSection.sourceId = sourceId;
    newSection.sourceSide = sourceSide;

    // connect all available sides
    for(uint8_t side = 2; side < 21; side++)
    {
        if(side != sourceSide
                && brick.neighbors[side].inUse != 0)
        {
            newSection.edges[side].available = 1;
        }
    }

    // if node-brick, then connect side 22
    if(brick.nodePos >= 0
            && brick.isInputBrick == 0)
    {
        newSection.edges[22].available = 1;
    }

    // add edge-section to the databuffer
    getEdgeBlock(brick)[position] = newSection;
    assert(newSection.sourceSide != 0);
    assert(getEdgeBlock(brick)[position].sourceSide != 0);

    return position;
}

//==================================================================================================

}

#endif // BRICK_ITEM_METHODS_H
