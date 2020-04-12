/**
 *  @file    init_test.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "init_test.h"
#include <root_object.h>

#include <core/brick_handler.h>
#include <core/objects/brick.h>

#include <initializing/axon_initializer.h>
#include <initializing/network_initializer.h>

namespace KyoukoMind
{

InitTest::InitTest()
    : Kitsunemimi::CompareTestHelper("InitTest")
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
    m_network = new KyoukoMind::RootObject();
    RootObject::m_brickHandler = new BrickHandler();
    createNewNetwork(m_testBrickContent);
}

/**
 * @brief checkInit
 */
void InitTest::checkInit()
{
    std::vector<Brick*> bricks;
    uint32_t nodeNumberPerBrick = 10;

    uint64_t numberOfInitBrick = RootObject::m_brickHandler->getNumberOfBrick();
    TEST_EQUAL(numberOfInitBrick, 7);

    bricks.push_back(RootObject::m_brickHandler->getBrick(6));
    bricks.push_back(RootObject::m_brickHandler->getBrick(7));
    bricks.push_back(RootObject::m_brickHandler->getBrick(11));
    bricks.push_back(RootObject::m_brickHandler->getBrick(12));
    bricks.push_back(RootObject::m_brickHandler->getBrick(13));
    bricks.push_back(RootObject::m_brickHandler->getBrick(16));
    bricks.push_back(RootObject::m_brickHandler->getBrick(17));

    for(uint32_t i = 0; i < RootObject::m_brickHandler->getNumberOfBrick(); i++)
    {
        Brick* brick = RootObject::m_brickHandler->getBrickByIndex(i);

        for(uint32_t side = 0; side < 22; side++)
        {
            if(brick->neighbors[side].targetBrick != nullptr)
            {
                BrickID sourceId = brick->brickId;
                const Brick* targetBrick = brick->neighbors[side].targetBrick;
                BrickID compareSource = targetBrick->neighbors[23 - side].targetBrick->brickId;

                TEST_EQUAL(compareSource, sourceId);
            }
        }
    }

    Brick totalData(0,0,0);

    for(uint32_t i = 0; i < bricks.size(); i++)
    {
        const uint32_t itemCount = bricks.at(i)->dataConnections[EDGE_DATA].numberOfItems;
        const uint32_t blockCount = bricks.at(i)->dataConnections[EDGE_DATA].buffer.numberOfBlocks;
        totalData.dataConnections[EDGE_DATA].numberOfItems += itemCount;
        totalData.dataConnections[EDGE_DATA].buffer.numberOfBlocks += blockCount;
    }

    TEST_EQUAL((int)totalData.dataConnections[EDGE_DATA].numberOfItems, (nodeNumberPerBrick*6));
    TEST_EQUAL((int)totalData.dataConnections[EDGE_DATA].buffer.numberOfBlocks, 7);
}

/**
 * @brief cleanupTestCase
 */
void InitTest::cleanupTestCase()
{
    delete m_networkInitializer;
}

} // namespace KyoukoMind
