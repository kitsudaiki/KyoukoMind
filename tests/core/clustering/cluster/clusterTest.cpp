/**
 *  @file    clusterTest.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "clusterTest.h"

#include <core/clustering/cluster/cluster.h>
#include <core/structs/messageHeader.h>

#include <core/messaging/incomingMessageBuffer.h>
#include <core/messaging/outgoingMessageBuffer.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{

ClusterTest::ClusterTest() : Kitsune::CommonTest("ClusterTest")
{
    initTestCase();
    checkInitializing();
    checkNeighborHandling();
    checkBlocks();
    cleanupTestCase();
}

void ClusterTest::initTestCase()
{
    m_cluster = new Cluster(1337, 0,0,"/tmp/", 8, 8);
}

/**
 * @brief ClusterTest::checkInitializing
 */
void ClusterTest::checkInitializing()
{
    UNITTEST(m_cluster->getMetaData().clusterId, 1337)
    UNITTEST(m_cluster->getClusterType(), EMPTY_CLUSTER)
}

void ClusterTest::checkNeighborHandling()
{
    UNITTEST(m_cluster->setNeighbor(5, 1337), true)
    UNITTEST(m_cluster->setNeighbor(42, 1337), false)

    Neighbor* neighbor = m_cluster->getNeighbors() + 5;
    UNITTEST(neighbor->targetClusterId, 1337)
    UNITTEST(neighbor->targetSide, 16-5)
    UNITTEST(m_cluster->getNeighborId(5), 1337)
}

void ClusterTest::checkBlocks()
{
    #ifdef RUN_UNIT_TEST
    UNITTEST(m_cluster->initStaticBlocks(8), true)
    UNITTEST(m_cluster->initStaticBlocks(8), false)
    UNITTEST(m_cluster->m_metaData.numberOfStaticItems, 8)
    UNITTEST(m_cluster->m_metaData.numberOfStaticBlocks, 1)

    UNITTEST(m_cluster->initDynamicBlocks(8), true)
    UNITTEST(m_cluster->initDynamicBlocks(8), false)
    UNITTEST(m_cluster->m_metaData.numberOfDynamicItems, 8)
    UNITTEST(m_cluster->m_metaData.numberOfDynamicBlocks, 1)

    UNITTEST(m_cluster->reserveDynamicItem(), 8)
    UNITTEST(m_cluster->reserveDynamicItem(), 9)

    UNITTEST(m_cluster->deleteDynamicItem(10), false)
    UNITTEST(m_cluster->deleteDynamicItem(2), true)
    UNITTEST(m_cluster->deleteDynamicItem(8), true)
    UNITTEST(m_cluster->deleteDynamicItem(5), true)
    UNITTEST(m_cluster->deleteDynamicItem(8), false)

    UNITTEST(m_cluster->m_metaData.bytePositionOfNextEmptyBlock, 16)
    UNITTEST(m_cluster->m_metaData.bytePositionOfLastEmptyBlock, 40)

    UNITTEST(m_cluster->reuseItemPosition(), 16)
    UNITTEST(m_cluster->reuseItemPosition(), 64)
    UNITTEST(m_cluster->reuseItemPosition(), 40)
    UNITTEST(m_cluster->reuseItemPosition(), UNINIT_STATE_32)

    UNITTEST(m_cluster->m_metaData.bytePositionOfNextEmptyBlock, UNINIT_STATE_32)
    UNITTEST(m_cluster->m_metaData.bytePositionOfLastEmptyBlock, UNINIT_STATE_32)
    #endif
}

void ClusterTest::cleanupTestCase()
{
    delete m_cluster;
}

}
