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

inline SynapseSection*
getSynapseSectionBlock(const DataConnection* data)
{
    return (SynapseSection*)data->buffer.data;
}

inline EdgeSection*
getEdgeBlock(const DataConnection* data)
{
    return (EdgeSection*)data->buffer.data;
}

// init blocks
bool initDataBlocks(Brick* brick,
                    const uint8_t connectionId,
                    const uint32_t numberOfItems,
                    const uint32_t itemSize);

// init lists
bool initNodeBlocks(Brick* brick,
                    uint32_t numberOfNodes);
bool initSynapseSectionBlocks(Brick* brick,
                              const uint32_t numberOfSynapseSections);
bool initEdgeSectionBlocks(Brick *brick,
                           const uint32_t numberOfEdgeSections);


// delete items
bool deleteDynamicItem(Brick *brick,
                       const uint8_t connectionId,
                       const uint32_t itemPos);
uint32_t reuseItemPosition(Brick *brick,
                           const uint8_t connectionId);
uint32_t reserveDynamicItem(Brick* brick,
                            const uint8_t connectionId);


// expand lists
bool addSynapse(Brick* brick,
                const uint32_t synapseSectionId,
                const Synapse &newSynapse);
uint32_t addEmptySynapseSection(Brick* brick,
                                const uint32_t sourceId);
uint32_t addEmptyEdgeSection(Brick* brick,
                             const uint8_t sourceSide,
                             const uint32_t sourceId);

}

#endif // BUFFERCONTROLMETHODS_H
