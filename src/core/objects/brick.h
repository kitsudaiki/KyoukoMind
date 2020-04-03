/**
 *  @file    brick.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef BRICKMETA_H
#define BRICKMETA_H

#include <common.h>
#include <root_object.h>

#include <core/objects/brick_pos.h>
#include <core/objects/edges.h>
#include <core/objects/synapses.h>
#include <core/objects/node.h>
#include <core/objects/data_connection.h>
#include <core/objects/neighbor.h>
#include <core/objects/empty_placeholder.h>

namespace KyoukoMind
{

struct GlobalValues;

//==================================================================================================

inline bool isReady(Brick &brick);
inline void updateBufferData(Brick &brick);

//==================================================================================================

struct Brick
{
    // common
    BrickID brickId = UNINIT_STATE_32;
    BrickPos brickPos;
    uint8_t inQueue = 0;

    DataBuffer headerBuffer;

    // 0 - 23: neighbor-bricks
    // 24: the current brick
    Neighbor neighbors[25];
    uint8_t isOutputBrick = 0;
    uint8_t isInputBrick = 0;

    // data
    DataConnection dataConnections[3];
    float learningOverride = 0.5;
    GlobalValues globalValues;

    float outBuffer[10];
    uint8_t outBufferPos = 0;

    Brick(const BrickID &brickId,
          const uint32_t x,
          const uint32_t y)
    {
        this->brickId = brickId;
        this->brickPos.x1 = x;
        this->brickPos.y = y;

        for(uint8_t side = 0; side < 24; side++)
        {
            this->neighbors[side].targetSide = 23 - side;
        }

        updateBufferData(*this);
    }

    ~Brick()
    {
    }
} __attribute__((packed));

//==================================================================================================

/**
 * check all incoming buffer of the brick
 *
 * @return true if ready, else false
 */
inline bool
isReady(Brick &brick)
{
    for(uint8_t i = 0; i < 25; i++)
    {
        if(brick.neighbors[i].inUse == 1
                && brick.neighbors[i].nextBuffer == nullptr)
        {
            return false;
        }
    }

    for(uint8_t i = 0; i < 25; i++)
    {
        if(brick.neighbors[i].inUse == 1) {
            switchBuffer(brick.neighbors[i]);
        }
    }

    return true;
}

//==================================================================================================

/**
 * write the current mata-data to the buffer and the file
 */
inline void
updateBufferData(Brick &brick)
{
    uint32_t size = sizeof(Brick);
    memcpy(brick.headerBuffer.data, &brick, size);
    // TODO: readd persist meta-data-changes
}

//==================================================================================================

} // namespace KyoukoMind

#endif // BRICKMETA_H
