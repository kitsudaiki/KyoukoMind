/**
 *  @file    commonBrickMethods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "commonBrickMethods.h"

#include <buffering/commonDataBuffer.h>
#include <buffering/commonDataBufferMethods.h>

#include <kyoukoNetwork.h>
#include <core/messaging/messageBlockBuffer.h>

#include <core/bricks/brickObjects/brick.h>

namespace KyoukoMind
{

/**
 * @brief addClientConnection
 * @param brick
 * @param input
 * @param output
 * @return
 */
bool
addClientConnection(Brick *brick,
                    bool input,
                    bool output)
{
    if(brick == nullptr) {
        return false;
    }

    if(input)
    {
        brick->isInputBrick = 1;
        initNeighbor(brick, 24);
    }

    if(output)
    {
        DataConnection* data = &brick->dataConnections[NODE_DATA];
        if(data->inUse == 0) {
            return false;
        }

        brick->isOutputBrick = 1;
        // process nodes
        Node* start = (Node*)data->buffer.data;
        Node* end = start + data->numberOfItems;

        // iterate over all nodes in the brick
        for(Node* node = start;
            node < end;
            node++)
        {
            node->border = 1000.0f;
        }
    }

    return true;
}

/**
 * @brief getSummedValue
 * @param brick
 * @return
 */
float
getSummedValue(Brick* brick)
{
    DataConnection* data = &brick->dataConnections[NODE_DATA];
    if(data->buffer.data == nullptr) {
        return 0.0f;
    }

    // process nodes
    Node* start = (Node*)data->buffer.data;
    Node* end = start + data->numberOfItems;

    // iterate over all nodes in the brick
    float sum = 0.0f;
    for(Node* node = start;
        node < end;
        node++)
    {
        sum += node->currentState;
    }

    brick->outBuffer[brick->outBufferPos] = sum;
    brick->outBufferPos = (brick->outBufferPos + 1) % 10;

    float result = 0.0f;
    for(uint32_t i = 0; i < 10; i++)
    {
        result += brick->outBuffer[i];
    }
    result /= 10.0f;
    return result;
}

/**
 * @brief connectBricks
 * @param sourceBrick
 * @param sourceSide
 * @param targetBrick
 * @return
 */
bool
connectBricks(Brick* sourceBrick,
              const uint8_t sourceSide,
              Brick* targetBrick)
{
    // check if side is valid
    if(sourceSide >= 24
            || sourceBrick == nullptr
            || targetBrick == nullptr)
    {
        return false;
    }

    Neighbor* sourceNeighbor = &sourceBrick->neighbors[sourceSide];
    Neighbor* targetNeighbor = &targetBrick->neighbors[23-sourceSide];

    if(sourceNeighbor->inUse == 1
            || targetNeighbor->inUse == 1)
    {
        return false;
    }

    // add the new neighbor
    initNeighbor(sourceBrick, sourceSide, targetBrick->brickId);
    initNeighbor(targetBrick, 23-sourceSide, sourceBrick->brickId);

    return true;
}

/**
 * @brief disconnectBricks
 * @param brick
 * @param side
 * @return
 */
bool
disconnectBricks(Brick* sourceBrick,
                 const uint8_t sourceSide,
                 Brick* targetBrick)
{
    // check if side is valid
    if(sourceSide >= 24
            || sourceBrick == nullptr
            || targetBrick == nullptr)
    {
        return false;
    }

    Neighbor* sourceNeighbor = &sourceBrick->neighbors[sourceSide];
    Neighbor* targetNeighbor = &targetBrick->neighbors[23-sourceSide];

    if(sourceNeighbor->inUse == 0
            || targetNeighbor->inUse == 0)
    {
        return false;
    }

    // add the new neighbor
    uninitNeighbor(sourceBrick, sourceSide);
    uninitNeighbor(targetBrick, 23-sourceSide);

    return true;
}

/**
 * @brief initNeighbor
 * @param souceSide
 * @param targetBrickId
 */
bool
initNeighbor(Brick* brick,
             const uint8_t sourceSide,
             const uint32_t targetBrickId)
{
    // precheck
    if(brick == nullptr) {
        return false;
    }

    // get and check neighbor
    Neighbor* neighbor = &brick->neighbors[sourceSide];
    if(neighbor->inUse == 1) {
        return false;
    }
    neighbor->inUse = 1;

    // init neighbor
    if(sourceSide < 24)
    {
        neighbor->targetSide = 23 - sourceSide;
        neighbor->targetBrickId = targetBrickId;
        neighbor->outgoBuffer.targetSide = 23 - sourceSide;
        neighbor->outgoBuffer.targetBrickId = targetBrickId;
    }
    else
    {
        neighbor->targetSide = 24;
        neighbor->targetBrickId = brick->brickId;
        neighbor->outgoBuffer.targetSide = 24;
        neighbor->outgoBuffer.targetBrickId = brick->brickId;
    }

    // init message in outgoing buffer
    OutgoingBuffer* outgoBuffer = &neighbor->outgoBuffer;
    outgoBuffer->message = KyoukoNetwork::m_internalMessageBuffer->reserveBuffer();
    outgoBuffer->message->isLast = 1;
    outgoBuffer->initMessage();
    assert(outgoBuffer->message->type != UNDEFINED_MESSAGE);

    // set initial dummy message
    neighbor->incomBuffer.addMessage(UNINIT_STATE_64-1);

    // write brick-metadata to buffer
    brick->updateBufferData();
    return true;
}

/**
 * @brief uninitNeighbor
 * @param side
 */
bool
uninitNeighbor(Brick* brick,
               const uint8_t side)
{
    Neighbor* neighbor = &brick->neighbors[side];
    if(neighbor->inUse == 0) {
        return false;
    }

    neighbor->inUse = 0;

    brick->updateBufferData();
    return true;
}

}
