/**
 *  @file    brickHandler.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/bricks/brick_handler.h>
#include <core/bricks/global_values_handler.h>

#include <core/bricks/brick_objects/brick.h>
#include <core/bricks/brick_methods/common_brick_methods.h>

namespace KyoukoMind
{

BrickHandler::BrickHandler()
{
}

BrickHandler::~BrickHandler()
{
    clearAllBrick();
}

/**
 * add a new brick to the handler
 *
 * @return false, if brick-id already exist in the handler, else true
 */
bool
BrickHandler::addBrick(const BrickID brickId,
                       Brick *brick)
{
    // check if id already in use
    if(m_allBricks.find(brickId) != m_allBricks.end()) {
        return false;
    }

    // add brick to handler and initial add to the processing-queue
    m_allBricks.insert(std::pair<BrickID, Brick*>(brickId, brick));
    addToQueue(brick);

    return true;
}

/**
 * request a specific brick from the handler
 *
 * @return null-pointer, if id is unknown, else pointer to the brick-object
 */
Brick*
BrickHandler::getBrick(const BrickID brickId)
{
    std::map<BrickID, Brick*>::iterator it;
    it = m_allBricks.find(brickId);
    if(it != m_allBricks.end()) {
        return it->second;
    }

    return nullptr;
}

/**
 * request brick from the handler by position in the handler-memory instead of its id
 *
 * @return null-pointer, if index is too big, else pointer to the brick-object
 */
Brick*
BrickHandler::getBrickByIndex(const uint32_t index)
{
    // check index
    if(index >= m_allBricks.size()) {
        return nullptr;
    }

    // iterate over all bricks until index is reached
    uint32_t counter = 0;
    std::map<BrickID, Brick*>::iterator it;
    for(it = m_allBricks.begin(); it != m_allBricks.end(); ++it)
    {
        if(counter == index) {
            return it->second;
        }
        counter++;
    }

    return nullptr;
}

/**
 * get number of registered bricks
 *
 * @return number of bricks
 */
uint64_t
BrickHandler::getNumberOfBrick() const
{
    return m_allBricks.size();
}

/**
 * delete a specific brick by its id
 *
 * @return false if id doesn't exist, else true
 */
bool
BrickHandler::deleteBrick(const BrickID brickId)
{
    std::map<BrickID, Brick*>::iterator it;
    it = m_allBricks.find(brickId);
    if(it != m_allBricks.end()) {
        m_allBricks.erase(it);
        return true;
    }

    return false;
}

/**
 * delete all bricks from the handler
 */
void
BrickHandler::clearAllBrick()
{
    std::map<BrickID, Brick*>::iterator it;
    for(it = m_allBricks.begin(); it != m_allBricks.end(); ++it)
    {
        Brick* tempBrick = it->second;
        delete tempBrick;
    }
    m_allBricks.clear();
}

/**
 * connect two bricks from the handler with each other at a specific side
 *
 * @return result of the sub-call
 */
bool
BrickHandler::connect(const BrickID sourceBrickId,
                      const uint8_t sourceSide,
                      const BrickID targetBrickId)
{
    Brick* sourceBrick = getBrick(sourceBrickId);
    Brick* targetBrick = getBrick(targetBrickId);

    return connectBricks(sourceBrick, sourceSide, targetBrick);
}

/**
 * disconnect two bricks from the handler from each other
 *
 * @return result of the sub-call
 */
bool
BrickHandler::disconnect(const BrickID sourceBrickId,
                         const uint8_t sourceSide,
                         const BrickID targetBrickId)
{
    Brick* sourceBrick = getBrick(sourceBrickId);
    Brick* targetBrick = getBrick(targetBrickId);

    return disconnectBricks(sourceBrick, sourceSide, targetBrick);
}

/**
 * add a brick to the queue of bricks with are ready for processing
 *
 * @return false if already in the queue, else true
 */
bool
BrickHandler::addToQueue(Brick *brick)
{
    // precheck
    if(brick->inQueue == 1) {
        return false;
    }

    while (m_queueLock.test_and_set(std::memory_order_acquire)) {
        asm("");
    }

    // add to queue
    brick->inQueue = 1;
    assert(brick->isReady() == true);
    m_readyBricks.push(brick);

    m_queueLock.clear(std::memory_order_release);

    return true;
}

/**
 * get the next brick from the queue
 *
 * @return nullpointer, if queue is empty, else pointer to the brick
 */
Brick*
BrickHandler::getFromQueue()
{
    Brick* result = nullptr;
    while (m_queueLock.test_and_set(std::memory_order_acquire)) {
        asm("");
    }

    // force the processing-unit into wait state for one cycle
    m_activeCounter++;
    if(m_activeCounter == m_allBricks.size())
    {
        m_activeCounter = 0;
        m_queueLock.clear(std::memory_order_release);
        return nullptr;
    }

    // get the next from the queue
    if(m_readyBricks.empty() == false)
    {
        result = m_readyBricks.front();
        m_readyBricks.pop();
        result->inQueue = 0;
        assert(result->isReady() == true);
    }

    m_queueLock.clear(std::memory_order_release);

    return result;
}

} // namespace KyoukoMind
