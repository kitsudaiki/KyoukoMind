/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef BRICK_H
#define BRICK_H

#include <common.h>
#include <kyouko_root.h>

#include <core/processing/objects/edges.h>

namespace KyoukoMind
{

class Brick
{

public:

    //----------------------------------------------------------------------------------------------
    struct BrickPos
    {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;
        uint32_t w = 0;

    } __attribute__((packed));
    //----------------------------------------------------------------------------------------------

    // common
    uint32_t brickId = UNINIT_STATE_32;
    uint32_t nodeBrickId = UNINIT_STATE_32;

    BrickPos brickPos;
    uint8_t isOutputBrick = 0;
    uint8_t isInputBrick = 0;
    uint8_t isNodeBrick = 0;

    // 0 - 21: neighbor-bricks
    // 22: the current brick
    uint32_t neighbors[23];
    uint32_t nodePos = UNINIT_STATE_32;


    //----------------------------------------------------------------------------------------------

    Brick();
    Brick(const uint32_t &brickId,
          const uint32_t x,
          const uint32_t y);
    ~Brick();

    float getSummedValue(NetworkSegment &segment);

    bool connectBricks(const uint8_t sourceSide,
                       Brick &targetBrick);
    bool disconnectBricks(const uint8_t sourceSide);

    void writeMonitoringOutput(DataBuffer &buffer);
    void writeClientOutput(NetworkSegment &segment,
                           DataBuffer &buffer);

private:
    void initNeighbor(const uint8_t side,
                      uint32_t targetBrickId);
    bool uninitNeighbor(const uint8_t side);
};

}

#endif // BRICK_H
