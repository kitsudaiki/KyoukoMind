/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "brick.h"

#include <core/object_handling/network_segment.h>
#include <core/processing/objects/node.h>

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
Brick::getRandomNeighbor(const uint32_t lastBrick)
{
    const uint32_t random = rand() % 1024;

    for(uint32_t i = 0; i < 23; i++)
    {
        const uint8_t pos = (i + random) % 23;
        if(neighbors[pos] != UNINIT_STATE_32
                && neighbors[pos] != lastBrick)
        {
            return pos;
        }
    }

    return lastBrick;
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
