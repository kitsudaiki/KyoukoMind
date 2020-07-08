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
Brick::Brick(const uint32_t &brickId,
             const uint32_t x,
             const uint32_t y)
{
    this->brickId = brickId;
    this->brickPos.x = x;
    this->brickPos.y = y;

    initRandValues();
}

/**
 * @brief Brick::~Brick
 */
Brick::~Brick()
{
    delete randValue;
    delete randWeight;
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
    assert(nodePos >= 0);

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
    GlobalValues globalValue = KyoukoRoot::m_globalValuesHandler->getGlobalValues();

    // fill message
    Kitsunemimi::Kyouko::MonitoringMessage monitoringMessage;
    monitoringMessage.brickId = brickId;
    monitoringMessage.xPos = brickPos.x;
    monitoringMessage.yPos = brickPos.y;

    // edges
    // monitoringMessage.numberOfEdgeSections = edges.numberOfItems
    //                                         - edges.numberOfDeletedDynamicItems;

    monitoringMessage.globalLearning = globalValue.globalLearningOffset;
    monitoringMessage.globalMemorizing = globalValue.globalMemorizingOffset;

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
 * connect two bricks by initialing the neighbors betweens the two bricks
 *
 * @return true, if successful, else false
 */
bool
Brick::connectBricks(const uint8_t sourceSide,
                     Brick &targetBrick)
{
    assert(sourceSide < 23);

    // get neighbor-pointers
    Neighbor* sourceNeighbor = &neighbors[sourceSide];
    Neighbor* targetNeighbor = &targetBrick.neighbors[23-sourceSide];

    // check neighbors
    if(sourceNeighbor->inUse == 1
            || targetNeighbor->inUse == 1)
    {
        return false;
    }

    // init the new neighbors
    this->initBrickNeighbor(sourceSide,
                            &targetBrick,
                            targetNeighbor);
    targetBrick.initBrickNeighbor(23 - sourceSide,
                                  this,
                                  sourceNeighbor);

    return true;
}

/**
 * remove the connection between two neighbors
 *
 * @return true, if successful, else false
 */
bool
Brick::disconnectBricks(const uint8_t sourceSide,
                        Brick &targetBrick)
{
    assert(sourceSide < 23);

    // get neighbor-pointers
    Neighbor* sourceNeighbor = &neighbors[sourceSide];
    Neighbor* targetNeighbor = &targetBrick.neighbors[23 - sourceSide];

    // check neighbors
    if(sourceNeighbor->inUse == 0
            || targetNeighbor->inUse == 0)
    {
        return false;
    }

    // add the new neighbor
    this->uninitBrickNeighbor(sourceSide);
    targetBrick.uninitBrickNeighbor(23 - sourceSide);

    return true;
}

/**
 * @brief initRandValues
 * @param brick
 */
void
Brick::initRandValues()
{
    randWeight = new float[999];
    float compare = 0.0f;
    for(uint32_t i = 0; i < 999; i++)
    {
        if(i % 3 == 0) {
            compare = 0.0f;
        }

        float tempValue = static_cast<float>(rand()) / 0x7FFFFFFF;
        assert(tempValue <= 1.0f);
        if(tempValue + compare > 1.0f) {
            tempValue = 1.0f - compare;
        }
        compare += tempValue;
        randWeight[i] = tempValue;
    }

    randValue = new uint32_t[1024];
    for(uint32_t i = 0; i < 1024; i++)
    {
        randValue[i] = static_cast<uint32_t>(rand());
    }
}

/**
 * initialize a specific neighbor of a brick
 *
 * @return true, if successful, else false
 */
bool
Brick::initBrickNeighbor(const uint8_t sourceSide,
                         Brick* targetBrick,
                         Neighbor* targetNeighbor)
{

    // get and check neighbor
    Neighbor* neighbor = &neighbors[sourceSide];
    assert(neighbor->inUse == 0);

    // init neighbor
    initNeighbor(*neighbor, targetBrick, targetNeighbor);

    return true;
}

/**
 * uninitialize a specific neighbor of a brick
 *
 * @return true, if successful, else false
 */
bool
Brick::uninitBrickNeighbor(const uint8_t side)
{
    // get and check neighbor
    Neighbor* neighbor = &neighbors[side];
    assert(neighbor->inUse == 1);

    // uninit
    // TODO: issue #58
    neighbor->inUse = 0;

    return true;
}

/**
 * @brief Brick::initNeighbor
 * @param neighbor
 * @param targetBrick
 * @param targetNeighbor
 */
void
Brick::initNeighbor(Neighbor &neighbor,
                    Brick* targetBrick,
                    Neighbor* targetNeighbor)
{
    assert(neighbor.inUse == 0);

    // init side
    neighbor.targetNeighbor = targetNeighbor;
    neighbor.targetBrick = targetBrick;
    neighbor.inUse = 1;
}

} // namespace KyoukoMind
