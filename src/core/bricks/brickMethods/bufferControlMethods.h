/**
 *  @file    bufferControlMethods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef BUFFERCONTROLMETHODS_H
#define BUFFERCONTROLMETHODS_H

#include <common.h>
#include <core/bricks/brickObjects/brick.h>

namespace KyoukoMind
{

struct Brick;

// getter
inline Node*
getNodeBlock(DataConnection* data)
{
    return (Node*)data->buffer.data;
}

inline EdgeSection*
getEdgeSectionBlock(const DataConnection* data)
{
    return (EdgeSection*)data->buffer.data;
}

inline ForwardEdgeSection*
getForwardEdgeBlock(const DataConnection* data)
{
    return (ForwardEdgeSection*)data->buffer.data;
}

// init blocks
bool initDataBlocks(Brick* brick,
                    const uint8_t connectionId,
                    const uint32_t numberOfItems,
                    const uint32_t itemSize);

// init lists
bool initNodeBlocks(Brick* brick,
                    uint32_t numberOfNodes);
bool initEdgeSectionBlocks(Brick* brick,
                           const uint32_t numberOfEdgeSections);
bool initForwardEdgeSectionBlocks(Brick *brick,
                                  const uint32_t numberOfForwardEdgeSections);


// delete items
bool deleteDynamicItem(Brick *brick,
                       const uint8_t connectionId,
                       const uint32_t itemPos);
uint32_t reuseItemPosition(Brick *brick,
                           const uint8_t connectionId);
uint32_t reserveDynamicItem(Brick* brick,
                            const uint8_t connectionId);


// expand lists
bool addEdge(Brick* brick,
             const uint32_t edgeSectionId,
             const Edge &newEdge);
uint32_t addEmptyEdgeSection(Brick* brick,
                             const uint32_t sourceId);
uint32_t addEmptyForwardEdgeSection(Brick* brick,
                                    const uint8_t sourceSide,
                                    const uint32_t sourceId);

}

#endif // BUFFERCONTROLMETHODS_H
