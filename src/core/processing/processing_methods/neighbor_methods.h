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
sendNeighborBuffer(Neighbor &sourceNeighbor)
{
    while(sourceNeighbor.lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    while(sourceNeighbor.targetNeighbor->lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    assert(sourceNeighbor.outgoingBuffer != nullptr);

    sourceNeighbor.targetNeighbor->bufferQueue.push(sourceNeighbor.outgoingBuffer);
    sourceNeighbor.outgoingBuffer = nullptr;

    sourceNeighbor.targetNeighbor->lock.clear(std::memory_order_release);
    sourceNeighbor.lock.clear(std::memory_order_release);
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

    while(neighbor.lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    assert(neighbor.outgoingBuffer == nullptr);
    assert(neighbor.currentBuffer != nullptr);
    assert(neighbor.bufferQueue.size() > 0);

    neighbor.outgoingBuffer = neighbor.currentBuffer;
    resetBuffer(*neighbor.currentBuffer);
    neighbor.currentBuffer = neighbor.bufferQueue.front();
    neighbor.bufferQueue.pop();

    neighbor.lock.clear(std::memory_order_release);
}

//==================================================================================================

inline StackBuffer*
getCurrentBuffer(Neighbor &neighbor)
{
    StackBuffer* result = nullptr;

    while(neighbor.lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    assert(neighbor.currentBuffer != nullptr);
    result = neighbor.currentBuffer;

    neighbor.lock.clear(std::memory_order_release);

    return result;
}

//==================================================================================================

}

#endif // NEIGHBOR_METHODS_H
