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

#include <libKitsunemimiPersistence/logger/logger.h>

namespace KyoukoMind
{

struct GlobalValues;

//==================================================================================================

inline bool isBrickReady(Brick &brick);
inline void updateBrickBufferData(Brick &brick);

//==================================================================================================

struct Brick
{
    // common
    BrickID brickId = UNINIT_STATE_32;
    BrickPos brickPos;
    uint8_t inQueue = 0;
    uint8_t isOutputBrick = 0;
    uint8_t isInputBrick = 0;

    DataBuffer headerBuffer;

    // 0 - 21: neighbor-bricks
    // 22: the current brick
    Neighbor neighbors[23];
    uint32_t readyMask = 0;
    uint32_t readyStatus = 0;

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
        this->brickPos.x = x;
        this->brickPos.y = y;

        updateBrickBufferData(*this);
    }

    ~Brick()
    {
    }

} __attribute__((packed));

} // namespace KyoukoMind

#endif // BRICKMETA_H
