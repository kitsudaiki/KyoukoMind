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

inline bool isReady(Brick &brick);
inline void updateBufferData(Brick &brick);

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

        updateBufferData(*this);
    }

    ~Brick()
    {
    }

} __attribute__((packed));

//==================================================================================================

inline bool
isReady(Brick &brick)
{
    return brick.readyStatus == brick.readyMask;
}

//==================================================================================================

/**
 * @brief updateReadyStatus
 *
 * @param brick
 * @param side
 */
inline void
updateReadyStatus(Brick &brick, const uint8_t side)
{
    uint32_t pos = 0x1;
    if(brick.brickId == 22) {
        LOG_DEBUG("####################### side: " + std::to_string(side));
        LOG_DEBUG("####################### is: " + std::to_string(brick.readyStatus) + "   should: " + std::to_string(brick.readyMask));
    }
    brick.readyStatus = brick.readyStatus | (pos << side);
    if(brick.brickId == 22) {
        LOG_DEBUG("####################### is: " + std::to_string(brick.readyStatus) + "   should: " + std::to_string(brick.readyMask));

    }
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
