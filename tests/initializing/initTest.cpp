/**
 *  @file    initTest.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "initTest.h"
#include <kyoukoNetwork.h>

#include <core/bricks/brickHandler.h>
#include <core/bricks/brickObjects/brick.h>

#include <initializing/axonInitializer.h>
#include <initializing/networkInitializer.h>

namespace KyoukoMind
{

/**
 * @brief InitTest::InitTest
 */
InitTest::InitTest() : Kitsune::CommonTest("InitTest")
{
    initTestCase();
    checkInit();
    cleanupTestCase();
}

/**
 * @brief InitTest::initTestCase
 */
void InitTest::initTestCase()
{
    m_network = new KyoukoMind::KyoukoNetwork();
    KyoukoNetwork::m_brickHandler = new BrickHandler();
    createNewNetwork(m_testBrickContent);
}

/**
 * @brief InitTest::checkInit
 */
void InitTest::checkInit()
{
    std::vector<Brick*> bricks;
    uint32_t nodeNumberPerBrick = KyoukoNetwork::m_config->getNumberOfNodes();

    uint64_t numberOfInitBrick = KyoukoNetwork::m_brickHandler->getNumberOfBrick();
    UNITTEST(numberOfInitBrick, 7);

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
                BrickID compareSource = KyoukoNetwork::m_brickHandler->getBrick(targetId)->neighbors[23 - side].targetBrickId;
                UNITTEST(compareSource, sourceId);
            }
        }
    }

    Brick totalData(0,0,0);

    for(uint32_t i = 0; i < bricks.size(); i++)
    {
        totalData.dataConnections[FORWARDEDGE_DATA].numberOfItems += bricks.at(i)->dataConnections[FORWARDEDGE_DATA].numberOfItems;
        totalData.dataConnections[FORWARDEDGE_DATA].numberOfItemBlocks += bricks.at(i)->dataConnections[FORWARDEDGE_DATA].numberOfItemBlocks;
    }

    UNITTEST((int)totalData.dataConnections[FORWARDEDGE_DATA].numberOfItems, (nodeNumberPerBrick*6));
    UNITTEST((int)totalData.dataConnections[FORWARDEDGE_DATA].numberOfItemBlocks, 7);
}

/**
 * @brief InitTest::cleanupTestCase
 */
void InitTest::cleanupTestCase()
{
    delete m_networkInitializer;
}

}
