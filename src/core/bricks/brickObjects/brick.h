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
#include <kyoukoNetwork.h>
#include <buffering/commonDataBuffer.h>

#include <core/bricks/brickObjects/edges.h>
#include <core/bricks/brickObjects/node.h>

#include <core/messaging/messageMarker/incomingBuffer.h>
#include <core/messaging/messageMarker/outgoingBuffer.h>

namespace KyoukoMind
{

struct GlobalValues;

//==================================================================================================

struct BrickPos
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;

} __attribute__((packed));

//==================================================================================================

enum DataConnectionTypes
{
    EDGE_DATA = 0,
    NODE_DATA = 1,
    SYNAPSE_DATA = 2
};

//==================================================================================================

struct DataConnection
{
    uint8_t inUse = 0;
    uint32_t itemSize = 0;
    uint32_t numberOfItems = 0;
    uint32_t numberOfItemBlocks = 0;
    uint32_t numberOfDeletedDynamicItems = 0;
    Kitsune::CommonDataBuffer buffer;

    uint32_t bytePositionOfFirstEmptyBlock = UNINIT_STATE_32;
    uint32_t bytePositionOfLastEmptyBlock = UNINIT_STATE_32;
    uint32_t numberOfEmptyBlocks = 0;

} __attribute__((packed));

//==================================================================================================

struct Neighbor
{
    uint8_t inUse = 0;

    BrickID targetBrickId = UNINIT_STATE_32;
    BrickPos targetBrickPos;
    uint8_t targetSide = 0;

    OutgoingBuffer outgoBuffer;
    IncomingBuffer incomBuffer;

} __attribute__((packed));

//==================================================================================================

struct Brick
{
    // common
    BrickID brickId = UNINIT_STATE_32;
    BrickPos brickPos;
    uint8_t inQueue = 0;

    Kitsune::CommonDataBuffer headerBuffer;

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
        this->brickPos.x = x;
        this->brickPos.y = y;

        for(uint8_t side = 0; side < 24; side++)
        {
            this->neighbors[side].targetSide = 23 - side;
        }

        updateBufferData();
    }

    ~Brick()
    {
    }

    /**
     * check all incoming buffer of the brick
     *
     * @return true if ready, else false
     */
    bool isReady()
    {
        for(uint8_t i = 0; i < 25; i++)
        {
            if(neighbors[i].inUse == 1 && neighbors[i].incomBuffer.isReady() == false) {
                return false;
            }
        }
        return true;
    }

    /**
     * write the current mata-data to the buffer and the file
     */
    void updateBufferData()
    {
        return;
        uint32_t size = sizeof(Brick);
        memcpy(headerBuffer.data, this, size);
        // TODO: readd persist meta-data-changes
    }

} __attribute__((packed));

//==================================================================================================

struct EmptyPlaceHolder
{
    uint8_t status = DELETED_SECTION;
    uint32_t bytePositionOfNextEmptyBlock = UNINIT_STATE_32;

} __attribute__((packed));

//==================================================================================================

}

#endif // BRICKMETA_H
