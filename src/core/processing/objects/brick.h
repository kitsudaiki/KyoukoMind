/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef BRICK_H
#define BRICK_H

#include <common.h>
#include <kyouko_root.h>

#include <core/processing/objects/edges.h>

class Brick
{

public:
    //----------------------------------------------------------------------------------------------
    struct PossibleNext
    {
        uint8_t next[3];
    } __attribute__((packed));
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

    // 0 - 21: neighbor-bricks
    // 22: the current brick
    uint32_t neighbors[23];
    uint32_t nodePos = UNINIT_STATE_32;
    uint32_t outputPos = UNINIT_STATE_32;

    //----------------------------------------------------------------------------------------------

    Brick();
    Brick(const uint32_t &brickId,
          const uint32_t x,
          const uint32_t y);
    ~Brick();

    uint32_t getRandomNeighbor(const uint32_t location);

    bool connectBricks(const uint8_t sourceSide,
                       Brick &targetBrick);
    bool disconnectBricks(const uint8_t sourceSide);

private:
    void initNeighborList();

    void initNeighbor(const uint8_t side,
                      uint32_t targetBrickId);
    bool uninitNeighbor(const uint8_t side);

    const PossibleNext getPossibleNext(const uint8_t inputSide);
};

#endif // BRICK_H
