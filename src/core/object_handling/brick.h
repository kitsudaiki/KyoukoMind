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
    struct Neighbor
    {
        uint8_t inUse = 0;
        Brick* targetBrick = nullptr;
        Neighbor* targetNeighbor = nullptr;
        BrickPos targetBrickPos;
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
    Neighbor neighbors[23];
    uint32_t nodePos = UNINIT_STATE_32;


    //----------------------------------------------------------------------------------------------

    Brick(const uint32_t &brickId,
          const uint32_t x,
          const uint32_t y);
    ~Brick();

    float getSummedValue(NetworkSegment &segment);

    bool connectBricks(const uint8_t sourceSide,
                       Brick &targetBrick);
    bool disconnectBricks(const uint8_t sourceSide,
                          Brick &targetBrick);

    void writeMonitoringOutput(DataBuffer &buffer);
    void writeClientOutput(NetworkSegment &segment,
                           DataBuffer &buffer);

private:
    bool initBrickNeighbor(const uint8_t sourceSide,
                           Brick* targetBrick,
                           Neighbor* targetNeighbor);

    void initNeighbor(Neighbor &neighbor,
                      Brick* targetBrick,
                      Neighbor* targetNeighbor);
    bool uninitBrickNeighbor(const uint8_t side);
};

}

#endif // BRICK_H
