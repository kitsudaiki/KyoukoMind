/**
 *  @file    init_test.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "init_test.h"
#include <kyouko_network.h>

#include <core/bricks/brick_handler.h>
#include <core/bricks/brick_objects/brick.h>

#include <initializing/axon_initializer.h>
#include <initializing/network_initializer.h>

namespace KyoukoMind
{

InitTest::InitTest()
    : Kitsunemimi::Test("InitTest")
{
    initTestCase();
    checkInit();
    cleanupTestCase();
}

/**
 * @brief initTestCase
 */
void InitTest::initTestCase()
{
    m_network = new KyoukoMind::KyoukoNetwork();
    KyoukoNetwork::m_brickHandler = new BrickHandler();
    createNewNetwork(m_testBrickContent);
}

/**
 * @brief checkInit
 */
void InitTest::checkInit()
{
    std::vector<Brick*> bricks;
    uint32_t nodeNumberPerBrick = 10;

    uint64_t numberOfInitBrick = KyoukoNetwork::m_brickHandler->getNumberOfBrick();
    TEST_EQUAL(numberOfInitBrick, 7);

    bricks.push_back(KyoukoNetwork::m_brickHandler->getBrick(6));
    bricks.push_back(KyoukoNetwork::m_brickHandler->getBrick(7));
    bricks.push_back(KyoukoNetwork::m_brickHandler->getBrick(11));
    bricks.push_back(KyoukoNetwork::m_brickHandler->getBrick(12));
    bricks.push_back(KyoukoNetwork::m_brickHandler->getBrick(13));
    bricks.push_back(KyoukoNetwork::m_brickHandler->getBrick(16));
    bricks.push_back(KyoukoNetwork::m_brickHandler->getBrick(17));

    for(uint32_t i = 0; i < KyoukoNetwork::m_brickHandler->getNumberOfBrick(); i++)
    {
        Brick* brick = KyoukoNetwork::m_brickHandler->getBrickByIndex(i);

        for(uint32_t side = 0; side < 24; side++)
        {
            if(brick->neighbors[side].targetBrickId != UNINIT_STATE_32)
            {
                BrickID sourceId = brick->brickId;
                BrickID targetId = brick->neighbors[side].targetBrickId;

                const Brick* targetBrick = KyoukoNetwork::m_brickHandler->getBrick(targetId);
                BrickID compareSource = targetBrick->neighbors[23 - side].targetBrickId;

                TEST_EQUAL(compareSource, sourceId);
            }
        }
    }

    Brick totalData(0,0,0);

    for(uint32_t i = 0; i < bricks.size(); i++)
    {
        const uint32_t itemCount = bricks.at(i)->dataConnections[EDGE_DATA].numberOfItems;
        const uint32_t blockCount = bricks.at(i)->dataConnections[EDGE_DATA].numberOfItemBlocks;
        totalData.dataConnections[EDGE_DATA].numberOfItems += itemCount;
        totalData.dataConnections[EDGE_DATA].numberOfItemBlocks += blockCount;
    }

    TEST_EQUAL((int)totalData.dataConnections[EDGE_DATA].numberOfItems, (nodeNumberPerBrick*6));
    TEST_EQUAL((int)totalData.dataConnections[EDGE_DATA].numberOfItemBlocks, 7);
}

/**
 * @brief cleanupTestCase
 */
void InitTest::cleanupTestCase()
{
    delete m_networkInitializer;
}

} // namespace KyoukoMind
