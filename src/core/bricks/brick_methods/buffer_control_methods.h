/**
 *  @file    buffer_control_methods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef BUFFER_CONTROL_METHODS_H
#define BUFFER_CONTROL_METHODS_H

#include <common.h>
#include <core/bricks/brick_objects/brick.h>

namespace KyoukoMind
{

// init blocks
bool initDataBlocks(Brick &brick,
                    const uint8_t connectionId,
                    const uint32_t numberOfItems,
                    const uint32_t itemSize);

// init lists
bool initNodeBlocks(Brick &brick,
                    uint32_t numberOfNodes);
bool initSynapseSectionBlocks(Brick &brick,
                              const uint32_t numberOfSynapseSections);
bool initEdgeSectionBlocks(Brick &brick,
                           const uint32_t numberOfEdgeSections);


// delete items
bool deleteDynamicItem(Brick &brick,
                       const uint8_t connectionId,
                       const uint32_t itemPos);
uint32_t reuseItemPosition(Brick &brick,
                           const uint8_t connectionId);
uint32_t reserveDynamicItem(Brick &brick,
                            const uint8_t connectionId);


// expand lists
bool addSynapse(Brick &brick,
                const uint32_t synapseSectionId,
                const Synapse &newSynapse);
uint32_t addEmptySynapseSection(Brick &brick,
                                const uint32_t sourceId);
uint32_t addEmptyEdgeSection(Brick &brick,
                             const uint8_t sourceSide,
                             const uint32_t sourceId);

} // namespace KyoukoMind

#endif // BUFFER_CONTROL_METHODS_H
