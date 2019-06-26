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
 * add a new client-connection to a brick,
 * for data input and output
 *
 * @return true, if successful, else false
 */
bool
addClientConnection(Brick *brick,
                    bool isInput,
                    bool isOutput)
{
    // precheck
    assert(brick == nullptr);

    // add input-connection
    if(isInput)
    {
        // set brick as input-brick
        brick->isInputBrick = 1;

        // init the incoming-message-queue
        // for incoming messages from the client
        initNeighbor(brick, 24);
    }

    // add output-connection
    if(isOutput)
    {
        // get and check connection-item
        DataConnection* data = &brick->dataConnections[NODE_DATA];
        if(data->inUse == 0) {
            return false;
        }

        // set brick as output-brick
        brick->isOutputBrick = 1;

        // set the border-value of all nodes within the brick
        // to a high-value, so the node can never become active
        Node* start = (Node*)data->buffer.data;
        Node* end = start + data->numberOfItems;
        for(Node* node = start;
            node < end;
            node++)
        {
            node->border = 100000.0f;
        }
    }

    return true;
}

/**
 * summarize the state of all nodes in a brick
 * and return the average value of the last two cycles
 * for a cleaner output
 *
 * @return summend value of all nodes of the brick
 */
float
getSummedValue(Brick* brick)
{
    // precheck
    assert(brick == nullptr);
    if(brick->isOutputBrick > 0) {
        return false;
    }

    // get and check connection-item
    DataConnection* data = &brick->dataConnections[NODE_DATA];
    if(data->buffer.data == nullptr) {
        return 0.0f;
    }

    // iterate over all nodes in the brick and
    // summarize the states of all nodes
    Node* start = (Node*)data->buffer.data;
    Node* end = start + data->numberOfItems;
    float sum = 0.0f;
    for(Node* node = start;
        node < end;
        node++)
    {
        sum += node->currentState;
    }

    // write value to the internal ring-buffer
    brick->outBuffer[brick->outBufferPos] = sum;
    brick->outBufferPos = (brick->outBufferPos + 1) % 10;

    // summarize the ring-buffer and get the average value
    float result = 0.0f;
    for(uint32_t i = 0; i < 10; i++)
    {
        result += brick->outBuffer[i];
    }
    result /= 10.0f;

    return result;
}

/**
 * connect two bricks by initialing the neighbors betweens the two bricks
 *
 * @return true, if successful, else false
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

    // get neighbor-pointers
    Neighbor* sourceNeighbor = &sourceBrick->neighbors[sourceSide];
    Neighbor* targetNeighbor = &targetBrick->neighbors[23-sourceSide];

    // check neighbors
    if(sourceNeighbor->inUse == 1
            || targetNeighbor->inUse == 1)
    {
        return false;
    }

    // init the new neighbors
    initNeighbor(sourceBrick, sourceSide, targetBrick->brickId);
    initNeighbor(targetBrick, 23-sourceSide, sourceBrick->brickId);

    return true;
}

/**
 * remove the connection between two neighbors
 *
 * @return true, if successful, else false
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

    // get neighbor-pointers
    Neighbor* sourceNeighbor = &sourceBrick->neighbors[sourceSide];
    Neighbor* targetNeighbor = &targetBrick->neighbors[23-sourceSide];

    // check neighbors
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
 * initialize a specific neighbor of a brick
 *
 * @return true, if successful, else false
 */
bool
initNeighbor(Brick* brick,
             const uint8_t sourceSide,
             const uint32_t targetBrickId)
{
    // precheck
    assert(brick != nullptr);

    // get and check neighbor
    Neighbor* neighbor = &brick->neighbors[sourceSide];
    if(neighbor->inUse == 1) {
        return false;
    }
    neighbor->inUse = 1;

    // init side
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

    // init outgoing buffer of the choosen side
    OutgoingBuffer* outgoBuffer = &neighbor->outgoBuffer;
    outgoBuffer->message = KyoukoNetwork::m_messageBuffer->reserveBuffer();
    outgoBuffer->message->isLast = 1;
    outgoBuffer->initMessage();
    assert(outgoBuffer->message->type != UNDEFINED_MESSAGE);

    // set initial dummy message to start the first cycle
    neighbor->incomBuffer.addMessage(UNINIT_STATE_64-1);

    // write brick-metadata to buffer
    brick->updateBufferData();

    return true;
}

/**
 * uninitialize a specific neighbor of a brick
 *
 * @return true, if successful, else false
 */
bool
uninitNeighbor(Brick* brick,
               const uint8_t side)
{
    // get and check neighbor
    Neighbor* neighbor = &brick->neighbors[side];
    if(neighbor->inUse == 0) {
        return false;
    }

    // uninit
    // TODO: issue #58
    neighbor->inUse = 0;

    // write brick-metadata to buffer
    brick->updateBufferData();

    return true;
}

}
