#ifndef BRICK_INITIALIZING_METHODS_H
#define BRICK_INITIALIZING_METHODS_H

#include <common.h>

#include <core/objects/brick.h>

namespace KyoukoMind
{

bool initBrickNeighbor(Brick &sourceBrick,
                       const uint8_t sourceSide,
                       Brick* targetBrick,
                       Neighbor* targetNeighbor);

bool uninitBrickNeighbor(Brick &sourceBrick,
                         const uint8_t side);

bool connectBricks(Brick &sourceBrick,
                   const uint8_t sourceSide,
                   Brick &targetBrick);

bool disconnectBricks(Brick &sourceBrick,
                      const uint8_t sourceSide,
                       Brick &targetBrick);

bool initDataBlocks(Brick &brick,
                    const uint8_t connectionId,
                    const uint32_t numberOfItems,
                    const uint32_t itemSize);

bool initNodeBlocks(Brick &brick,
                    uint32_t numberOfNodes);

bool initSynapseSectionBlocks(Brick &brick,
                              const uint32_t numberOfSynapseSections);

bool initEdgeSectionBlocks(Brick &brick,
                           const uint32_t numberOfEdgeSections);

bool addClientInputConnection(Brick &brick,
                              const uint32_t targetBrickId);

bool addClientOutputConnection(Brick &brick);

void initRandValues(Brick &brick);

}

#endif // BRICK_INITIALIZING_METHODS_H
