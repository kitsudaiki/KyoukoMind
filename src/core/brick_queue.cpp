/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <core/brick_queue.h>
#include <core/global_values_handler.h>

#include <core/object_handling/brick.h>

namespace KyoukoMind
{

BrickQueue::BrickQueue() {}

BrickQueue::~BrickQueue() {}

/**
 * @brief BrickQueue::setBorder
 * @param border
 */
void
BrickQueue::setBorder(const uint32_t border)
{
    m_border = border;
}

/**
 * add a brick to the queue of bricks with are ready for processing
 *
 * @return false if already in the queue, else true
 */
bool
BrickQueue::addToQueue(Brick *brick)
{
    // precheck
    if(brick->inQueue == 1
            || brick->isInputBrick != 0)
    {
        return false;
    }

    while (m_queueLock.test_and_set(std::memory_order_acquire)) {
        asm("");
    }

    // add to queue
    brick->inQueue = 1;
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
BrickQueue::getFromQueue()
{
    Brick* result = nullptr;
    while (m_queueLock.test_and_set(std::memory_order_acquire)) {
        asm("");
    }

    // force the processing-unit into wait state for one cycle
    m_activeCounter++;
    if(m_activeCounter == m_border)
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
    }

    m_queueLock.clear(std::memory_order_release);

    return result;
}

} // namespace KyoukoMind
