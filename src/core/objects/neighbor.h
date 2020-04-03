#ifndef NEIGHBOR_H
#define NEIGHBOR_H

#include <common.h>
#include <core/objects/brick_pos.h>

namespace KyoukoMind
{

//==================================================================================================

struct Neighbor
{
    uint8_t inUse = 0;

    BrickID targetBrickId = UNINIT_STATE_32;
    BrickPos targetBrickPos;
    uint8_t targetSide = 0;

    StackBuffer* outgoingBuffer = nullptr;
    StackBuffer* currentBuffer = nullptr;
    StackBuffer* nextBuffer = nullptr;

    std::atomic_flag lock = ATOMIC_FLAG_INIT;

} __attribute__((packed));

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
             const uint8_t sourceSide,
             const uint32_t targetBrickId)
{
    assert(neighbor.inUse == 0);

    // init side
    neighbor.targetSide = 28 - sourceSide;
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
inline void
sendBuffer(Neighbor &sourceNeighbor,
           Neighbor &targetNeighbor)
{
    while(sourceNeighbor.lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    while(targetNeighbor.lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    assert(targetNeighbor.nextBuffer != nullptr);
    targetNeighbor.nextBuffer = sourceNeighbor.outgoingBuffer;
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
switchBuffer(Neighbor &neighbor)
{
    while(neighbor.lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    neighbor.outgoingBuffer = neighbor.currentBuffer;
    resetBuffer(*neighbor.currentBuffer);
    neighbor.currentBuffer = neighbor.nextBuffer;
    neighbor.nextBuffer = nullptr;

    neighbor.lock.clear(std::memory_order_release);
}

//==================================================================================================

/**
 * @brief isReady
 * @param neighbor
 * @return
 */
inline bool
isReady(Neighbor &neighbor)
{
    bool result = false;

    while(neighbor.lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    result = neighbor.nextBuffer != nullptr;
    neighbor.lock.clear(std::memory_order_release);

    return result;
}

//==================================================================================================

}

#endif // NEIGHBOR_H
