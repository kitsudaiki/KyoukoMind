/**
 *  @file    brickHandler.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/bricks/brickHandler.h>
#include <core/bricks/globalValuesHandler.h>

#include <core/bricks/brickObjects/brick.h>
#include <core/bricks/brickMethods/commonBrickMethods.h>

namespace KyoukoMind
{

/**
 * @brief BrickManager::BrickManager
 */
BrickHandler::BrickHandler()
{
}

/**
 * @brief BrickManager::~BrickManager
 */
BrickHandler::~BrickHandler()
{
    clearAllBrick();
}

/**
 * @brief BrickManager::addBrick
 * @param brickId
 * @param brick
 * @return
 */
bool
BrickHandler::addBrick(const BrickID brickId,
                       Brick *brick)
{
    if(m_allBricks.find(brickId) != m_allBricks.end()) {
        return false;
    }
    m_allBricks.insert(std::pair<BrickID, Brick*>(brickId, brick));
    addToQueue(brick);

    return true;
}

/**
 * @brief BrickManager::getBrick
 * @param brickId
 * @return
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
 * @brief BrickHandler::getBrickByPos
 * @param brickPos
 * @return
 */
Brick*
BrickHandler::getBrickByIndex(const uint32_t index)
{
    if(index >= m_allBricks.size()) {
        return nullptr;
    }
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
 * @brief BrickHandler::getNumberOfBrick
 * @return
 */
uint64_t
BrickHandler::getNumberOfBrick() const
{
    return m_allBricks.size();
}

/**
 * @brief BrickManager::deleteBrick
 * @param brickId
 * @return
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
 * @brief BrickManager::clearAllBrick
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
 * @brief BrickHandler::connect
 * @param sourceBrickId
 * @param sourceSide
 * @param targetBrickId
 * @return
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
 * @brief BrickHandler::connect
 * @param sourceBrickId
 * @param sourceSide
 * @param targetBrickId
 * @return
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
 * @brief BrickHandler::addToQueue
 * @param brick
 */
bool
BrickHandler::addToQueue(Brick *brick)
{
    if(brick->inQueue == 1) {
        return false;
    }
    while (m_queueLock.test_and_set(std::memory_order_acquire)) {
        ; // spin
    }
    brick->inQueue = 1;
    assert(brick->isReady() == true);
    m_readyBricks.push(brick);
    m_queueLock.clear(std::memory_order_release);
    return true;
}

/**
 * @brief BrickHandler::getFromQueue
 * @return
 */
Brick*
BrickHandler::getFromQueue()
{
    Brick* result = nullptr;
    while (m_queueLock.test_and_set(std::memory_order_acquire)) {
        ; // spin
    }

    // force the processing-unit into wait state for one cycle
    m_activeCounter++;
    if(m_activeCounter == m_allBricks.size())
    {
        m_activeCounter = 0;
        m_queueLock.clear(std::memory_order_release);
        return nullptr;
    }

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

}
