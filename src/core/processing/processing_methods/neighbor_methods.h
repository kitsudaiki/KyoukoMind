#ifndef NEIGHBOR_METHODS_H
#define NEIGHBOR_METHODS_H

#include <common.h>
#include <core/objects/neighbor.h>

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
             const uint32_t targetBrickId,
             Neighbor* targetNeighbor)
{
    assert(neighbor.inUse == 0);

    // init side
    neighbor.targetNeighbor = targetNeighbor;
    neighbor.targetBrickId = targetBrickId;
    neighbor.outgoingBuffer = new StackBuffer();
    neighbor.currentBuffer = new StackBuffer();

    neighbor.inUse = 1;
}

//==================================================================================================

/**
 * @brief setNextBuffer
 * @param sourceNeighbor
 * @param targetNeighbor
 */
inline bool
sendNeighborBuffer(Neighbor &targetNeighbor,
                   Kitsunemimi::StackBuffer* buffer)
{
    while(targetNeighbor.lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    targetNeighbor.bufferQueue.push(buffer);
    targetNeighbor.lock.clear(std::memory_order_release);
    return true;
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
    while(sourceNeighbor.lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    while(targetNeighbor.lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    targetNeighbor.bufferQueue.push(sourceNeighbor.outgoingBuffer);
    sourceNeighbor.outgoingBuffer = nullptr;

    targetNeighbor.lock.clear(std::memory_order_release);
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

    neighbor.outgoingBuffer = neighbor.currentBuffer;
    resetBuffer(*neighbor.currentBuffer);
    neighbor.currentBuffer = neighbor.bufferQueue.front();
    neighbor.bufferQueue.pop();

    neighbor.lock.clear(std::memory_order_release);
}

//==================================================================================================

}

#endif // NEIGHBOR_METHODS_H
