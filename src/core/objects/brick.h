/**
 *  @file    brick.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#ifndef BRICKMETA_H
#define BRICKMETA_H

#include <common.h>
#include <kyouko_root.h>

#include <core/objects/brick_pos.h>
#include <core/objects/edges.h>
#include <core/objects/synapses.h>
#include <core/objects/node.h>
#include <core/objects/data_connection.h>
#include <core/objects/neighbor.h>
#include <core/objects/empty_placeholder.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiCommon/buffer/data_buffer.h>

namespace KyoukoMind
{

struct GlobalValues;

//==================================================================================================

struct Brick
{
    // common
    uint32_t brickId = UNINIT_STATE_32;
    uint32_t nodeBrickId = UNINIT_STATE_32;

    BrickPos brickPos;
    uint8_t inQueue = 0;
    uint8_t isOutputBrick = 0;
    uint8_t isInputBrick = 0;
    uint8_t isNodeBrick = 0;

    // random values
    float* randWeight = nullptr;
    uint32_t randWeightPos = 0;
    uint32_t* randValue = nullptr;
    uint32_t randValuePos = 0;

    // 0 - 21: neighbor-bricks
    // 22: the current brick
    Neighbor neighbors[23];
    std::atomic_flag lock = ATOMIC_FLAG_INIT;

    // data
    DataConnection edges;
    int32_t nodePos = -1;

    // learning metadata
    float learningOverride = 0.5;
    GlobalValues globalValues;

    Brick(const uint32_t &brickId,
          const uint32_t x,
          const uint32_t y)
    {
        this->brickId = brickId;
        this->brickPos.x = x;
        this->brickPos.y = y;
    }

    ~Brick()
    {
        delete randValue;
        delete randWeight;
    }

} __attribute__((packed));

//==================================================================================================

} // namespace KyoukoMind

#endif // BRICKMETA_H
