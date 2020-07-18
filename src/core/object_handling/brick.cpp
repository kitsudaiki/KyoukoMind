/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "brick.h"

#include <core/object_handling/network_segment.h>
#include <core/processing/objects/node.h>
#include <core/processing/objects/edges.h>

#include <libKitsunemimiKyoukoCommon/communication_structs/monitoring_contianer.h>
#include <libKitsunemimiKyoukoCommon/communication_structs/mind_container.h>

namespace KyoukoMind
{

/**
 * @brief Brick::Brick
 * @param brickId
 * @param x
 * @param y
 */
Brick::Brick()
{
    initNeighborList();
}

Brick::Brick(const uint32_t &brickId,
             const uint32_t x,
             const uint32_t y)
{
    this->brickId = brickId;
    this->brickPos.x = x;
    this->brickPos.y = y;

    initNeighborList();
}

/**
 * @brief Brick::~Brick
 */
Brick::~Brick() {}

/**
 * @brief Brick::getRandomNeighbor
 * @return
 */
uint32_t
Brick::getRandomNeighbor(const uint32_t location)
{
    uint8_t inputSide = getInputSide(location);
    if(inputSide == 0) {
        inputSide = 9;
    }
    const PossibleNext next = getPossibleNext(inputSide);

    const uint8_t nextSide = next.next[rand() % 3];
    uint32_t nextLocation = neighbors[nextSide];

    if(nextLocation != UNINIT_STATE_32) {
        nextLocation += static_cast<uint32_t>(23 - nextSide) << 24;
    }

    return nextLocation;
}

/**
 * summarize the state of all nodes in a brick
 * and return the average value of the last two cycles
 * for a cleaner output
 *
 * @return summend value of all nodes of the brick
 */
float
Brick::getSummedValue(NetworkSegment &segment)
{
    assert(isOutputBrick != 0);
    assert(nodePos != UNINIT_STATE_32);

    Node* node = &static_cast<Node*>(segment.nodes.buffer.data)[nodePos];
    return node->currentState;

    // write value to the internal ring-buffer
    /*brick.outBuffer[brick.outBufferPos] += node->currentState;
    brick.outBufferPos = (brick.outBufferPos + 1) % 10;
    node->currentState /= NODE_COOLDOWN;

    // summarize the ring-buffer and get the average value
    float result = 0.0f;
    for(uint32_t i = 0; i < 10; i++)
    {
        result += brick.outBuffer[i];
    }
    result /= 10.0f;

    return result;*/
}

/**
 * @brief reportStatus
 */
void
Brick::writeMonitoringOutput(DataBuffer &buffer)
{
    // fill message
    Kitsunemimi::Kyouko::MonitoringMessage monitoringMessage;
    monitoringMessage.brickId = brickId;
    monitoringMessage.xPos = brickPos.x;
    monitoringMessage.yPos = brickPos.y;

    // edges
    // monitoringMessage.numberOfEdgeSections = edges.numberOfItems
    //                                         - edges.numberOfDeletedDynamicItems;

    // monitoringMessage.globalLearning = globalValue.globalLearningOffset;
    // monitoringMessage.globalMemorizing = globalValue.globalMemorizingOffset;

    Kitsunemimi::addObject_DataBuffer(buffer, &monitoringMessage);
}

/**
 * @brief writeOutput
 * @param brick
 * @param buffer
 */
void
Brick::writeClientOutput(NetworkSegment &segment,
                         DataBuffer &buffer)
{
    Kitsunemimi::Kyouko::MindOutputData outputMessage;
    outputMessage.value = getSummedValue(segment);
    outputMessage.brickId = brickId;

    Kitsunemimi::addObject_DataBuffer(buffer, &outputMessage);
}

/**
 * @brief Brick::initNeighborList
 */
void
Brick::initNeighborList()
{
    for(uint8_t i = 0; i < 23; i++)
    {
        neighbors[i] = UNINIT_STATE_32;
    }
}

/**
 * connect two bricks by initialing the neighbors betweens the two bricks
 *
 * @return true, if successful, else false
 */
bool
Brick::connectBricks(const uint8_t sourceSide,
                     Brick &targetBrick)
{
    assert(sourceSide < 23);

    // check neighbors
    if(neighbors[sourceSide] != UNINIT_STATE_32
            || targetBrick.neighbors[23-sourceSide] != UNINIT_STATE_32)
    {
        return false;
    }

    // init the new neighbors
    this->initNeighbor(sourceSide,
                       targetBrick.brickId);
    targetBrick.initNeighbor(23 - sourceSide,
                             this->brickId);

    return true;
}

/**
 * remove the connection between two neighbors
 *
 * @return true, if successful, else false
 */
bool
Brick::disconnectBricks(const uint8_t sourceSide)
{
    assert(sourceSide < 23);
    const uint32_t targetId = neighbors[sourceSide];

    Brick* targetBrick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[targetId];

    // check neighbors
    if(neighbors[sourceSide] == UNINIT_STATE_32
            || targetBrick->neighbors[23-sourceSide] == UNINIT_STATE_32)
    {
        return false;
    }

    // add the new neighbor
    this->uninitNeighbor(sourceSide);
    targetBrick->uninitNeighbor(23 - sourceSide);

    return true;
}

/**
 * uninitialize a specific neighbor of a brick
 *
 * @return true, if successful, else false
 */
bool
Brick::uninitNeighbor(const uint8_t side)
{
    if(neighbors[side] == UNINIT_STATE_32) {
        return false;
    }

    neighbors[side] = UNINIT_STATE_32;

    return true;
}

/**
 * @brief Brick::getPossibleNext
 * @param inputSide
 * @return
 */
const Brick::PossibleNext
Brick::getPossibleNext(const uint8_t inputSide)
{
    PossibleNext next;

    switch(inputSide)
    {
        case 9:
        {
            next.next[0] = 11;
            next.next[1] = 13;
            next.next[2] = 14;
            break;
        }

        case 10:
        {
            next.next[0] = 12;
            next.next[1] = 13;
            next.next[2] = 14;
            break;
        }

        case 11:
        {
            next.next[0] = 9;
            next.next[1] = 12;
            next.next[2] = 13;
            break;
        }

        case 12:
        {
            next.next[0] = 10;
            next.next[1] = 11;
            next.next[2] = 14;
            break;
        }

        case 13:
        {
            next.next[0] = 9;
            next.next[1] = 10;
            next.next[2] = 11;
            break;
        }

        case 14:
        {
            next.next[0] = 9;
            next.next[1] = 10;
            next.next[2] = 12;
            break;
        }

        default:
        {
            next.next[0] = UNINIT_STATE_8;
            next.next[1] = UNINIT_STATE_8;
            next.next[2] = UNINIT_STATE_8;
        }
    }

    return next;
}

/**
 * @brief Brick::initNeighbor
 * @param neighbor
 * @param targetBrick
 * @param targetNeighbor
 */
void
Brick::initNeighbor(const uint8_t side,
                    uint32_t targetBrickId)
{
    neighbors[side] = targetBrickId;
}

} // namespace KyoukoMind
