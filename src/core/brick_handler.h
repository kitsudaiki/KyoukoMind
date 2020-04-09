/**
 *  @file    brick_handler.h
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

class BrickHandler
{
public:
    BrickHandler();
    ~BrickHandler();

    // add
    bool addBrick(const BrickID brickId,
                  Brick* brick);
    // getter
    Brick* getBrick(const BrickID brickId);
    Brick* getBrickByIndex(const uint32_t index);
    uint64_t getNumberOfBrick() const;
    DataItem* getMetadata();

    // delete
    bool deleteBrick(const BrickID brickId);
    void clearAllBrick();

    bool connect(const BrickID sourceBrickId,
                 const uint8_t sourceSide,
                 const BrickID targetBrickId);

    bool disconnect(const BrickID sourceBrickId,
                    const uint8_t sourceSide,
                    const BrickID targetBrickId);

    // processing-queue
    bool addToQueue(Brick* brick);
    Brick* getFromQueue();

private:
    std::map<BrickID, Brick*> m_allBricks;

    std::queue<Brick*> m_readyBricks;
    std::atomic_flag m_queueLock = ATOMIC_FLAG_INIT;
    uint32_t m_activeCounter = 0;
};

} // namespace KyoukoMind

#endif // BRICK_HANDLER_H
