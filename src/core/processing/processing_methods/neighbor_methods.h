#ifndef NEIGHBOR_METHODS_H
#define NEIGHBOR_METHODS_H

#include <common.h>
#include <core/objects/neighbor.h>
#include <core/objects/brick.h>

#include <libKitsunemimiPersistence/logger/logger.h>

namespace KyoukoMind
{

//==================================================================================================

/**
 * @brief initNeighbor
 * @param neighbor
 * @param sourceBrickId
 * @param sourceSide
 * @param targetBrickId
 * @return
 */
inline void
initNeighbor(Neighbor &neighbor,
             Brick* targetBrick,
             Neighbor* targetNeighbor)
{
    assert(neighbor.inUse == 0);

    // init side
    neighbor.targetNeighbor = targetNeighbor;
    neighbor.targetBrick = targetBrick;
    neighbor.outgoingBuffer = nullptr;
    neighbor.currentBuffer = new StackBuffer();
    neighbor.bufferQueue.push(new StackBuffer());
    neighbor.bufferQueue.push(new StackBuffer());

    neighbor.inUse = 1;
}

//==================================================================================================

/**
 * @brief setNextBuffer
 * @param sourceNeighbor
 * @param targetNeighbor
 */
inline void
sendNeighborBuffer(Neighbor &sourceNeighbor,
                   Neighbor &targetNeighbor)
{
    assert(sourceNeighbor.outgoingBuffer != nullptr);

    targetNeighbor.bufferQueue.push(sourceNeighbor.outgoingBuffer);
    sourceNeighbor.outgoingBuffer = nullptr;
}

//==================================================================================================

/**
 * @brief switchBuffer
 * @param neighbor
 */
inline void
switchNeighborBuffer(Neighbor &neighbor)
{
    if(neighbor.inUse == 0) {
        return;
    }

    assert(neighbor.outgoingBuffer == nullptr);
    assert(neighbor.currentBuffer != nullptr);
    assert(neighbor.bufferQueue.size() > 0);

    neighbor.outgoingBuffer = neighbor.currentBuffer;
    Kitsunemimi::reset_StackBuffer(*neighbor.currentBuffer);
    neighbor.currentBuffer = neighbor.bufferQueue.front();
    neighbor.bufferQueue.pop();
}

//==================================================================================================

}

#endif // NEIGHBOR_METHODS_H
