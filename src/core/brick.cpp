/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "brick.h"

#include <core/methods/data_connection_methods.h>
#include <core/network_segment.h>

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
 * @brief initCycle
 * @param brick
 */
void
Brick::initCycle()
{
    while(lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    for(uint8_t side = 0; side < 23; side++) {
        switchNeighborBuffer(neighbors[side]);
    }

    lock.clear(std::memory_order_release);
}

/**
 * @brief finishCycle
 * @param brick
 * @param monitoringMessage
 * @param clientMessage
 */
void
Brick::finishCycle()
{
    while(lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    // finish standard-neighbors
    for(uint8_t side = 0; side < 23; side++) {
        finishSide(side);
    }

    processReady();

    lock.clear(std::memory_order_release);
}

/**
 * @brief finishSide
 * @param brick
 * @param side
 */
void
Brick::finishSide(const uint8_t sourceSide)
{
    Neighbor* sourceNeighbor = &neighbors[sourceSide];
    if(sourceNeighbor->inUse == 0) {
        return;
    }

    Brick* targetBrick = sourceNeighbor->targetBrick;

    while(targetBrick->lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    // finish side
    sendNeighborBuffer(*sourceNeighbor, *sourceNeighbor->targetNeighbor);
    targetBrick->processReady();

    targetBrick->lock.clear(std::memory_order_release);
}

/**
 * @brief processReady
 * @param brick
 * @return
 */
bool
Brick::processReady()
{
    if(isReady()
            && inQueue == 0)
    {
        KyoukoRoot::m_queue->addToQueue(this);
        lock.clear(std::memory_order_release);
        return true;
    }

    return false;
}

/**
 * @brief updateReady
 * @param brick
 * @param side
 */
bool
Brick::isReady()
{
    for(uint8_t side = 0; side < 23; side++)
    {
        if(neighbors[side].inUse == 1
                && neighbors[side].bufferQueue.size() == 0)
        {
            return false;
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
Brick::getSummedValue(NetworkSegment &segment)
{
    assert(isOutputBrick != 0);
    assert(nodePos >= 0);

    Node* node = &segment.getNodeBlock()[nodePos];
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
    monitoringMessage.numberOfEdgeSections = edges.numberOfItems
                                             - edges.numberOfDeletedDynamicItems;

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
    neighbor.outgoingBuffer = nullptr;
    neighbor.currentBuffer = new StackBuffer();
    neighbor.bufferQueue.push(new StackBuffer());
    neighbor.bufferQueue.push(new StackBuffer());

    neighbor.inUse = 1;
}

/**
 * @brief setNextBuffer
 * @param sourceNeighbor
 * @param targetNeighbor
 */
void
Brick::sendNeighborBuffer(Neighbor &sourceNeighbor,
                          Neighbor &targetNeighbor)
{
    assert(sourceNeighbor.outgoingBuffer != nullptr);

    targetNeighbor.bufferQueue.push(sourceNeighbor.outgoingBuffer);
    sourceNeighbor.outgoingBuffer = nullptr;
}

/**
 * @brief switchBuffer
 * @param neighbor
 */
void
Brick::switchNeighborBuffer(Neighbor &neighbor)
{
    if(neighbor.inUse == 0) {
        return;
    }

    assert(neighbor.outgoingBuffer == nullptr);
    assert(neighbor.currentBuffer != nullptr);
    assert(neighbor.bufferQueue.size() > 0);

    neighbor.outgoingBuffer = neighbor.currentBuffer;
    Kitsunemimi::reset_StackBuffer(*neighbor.outgoingBuffer);
    neighbor.currentBuffer = neighbor.bufferQueue.front();
    neighbor.bufferQueue.pop();
}

} // namespace KyoukoMind
