#ifndef BRICK_INITIALIZING_METHODS_H
#define BRICK_INITIALIZING_METHODS_H

#include <common.h>

#include <core/objects/brick.h>
#include <core/objects/network_segment.h>

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

bool initEdgeSectionBlocks(Brick &brick,
                           const uint32_t numberOfEdgeSections);

void initRandValues(Brick &brick);

}

#endif // BRICK_INITIALIZING_METHODS_H
