/**
 *  @file    brick_queue.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef BRICK_HANDLER_H
#define BRICK_HANDLER_H

#include <common.h>

namespace KyoukoMind
{
class GlobalValuesHandler;

struct Brick;

class BrickQueue
{
public:
    BrickQueue();
    ~BrickQueue();

    void setBorder(const uint32_t border);

    // processing-queue
    bool addToQueue(Brick* brick);
    Brick* getFromQueue();

private:
    std::queue<Brick*> m_readyBricks;
    std::atomic_flag m_queueLock = ATOMIC_FLAG_INIT;
    uint32_t m_activeCounter = 0;
    uint32_t m_border = 0;
};

} // namespace KyoukoMind

#endif // BRICK_HANDLER_H
