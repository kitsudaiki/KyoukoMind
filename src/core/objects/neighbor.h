#ifndef NEIGHBOR_H
#define NEIGHBOR_H

#include <common.h>
#include <core/objects/brick_pos.h>

namespace KyoukoMind
{

struct Neighbor
{
    uint8_t inUse = 0;

    BrickID targetBrickId = UNINIT_STATE_32;
    Neighbor* targetNeighbor = nullptr;
    BrickPos targetBrickPos;

    std::queue<StackBuffer*> bufferQueue;
    StackBuffer* outgoingBuffer = nullptr;
    StackBuffer* currentBuffer = nullptr;

    std::atomic_flag lock = ATOMIC_FLAG_INIT;

} __attribute__((packed));

}

#endif // NEIGHBOR_H
