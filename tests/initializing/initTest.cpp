/**
 *  @file    initTestcpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "initTest.h"
#include <kyoChanNetwork.h>

#include <core/clustering/clusterHandler.h>
#include <core/clustering/clusterQueue.h>

#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <initializing/axonInitializer.h>
#include <initializing/clusterInitilizer.h>
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
    m_testClusterHandler = new ClusterHandler();
    m_networkInitializer = new NetworkInitializer(m_testClusterContent,
                                                  "/tmp/",
                                                  m_testClusterHandler);
    m_networkInitializer->initNetwork();
}

/**
 * @brief InitTest::checkInit
 */
void InitTest::checkInit()
{
    std::vector<ClusterMetaData> metaData;
    uint32_t nodeNumberPerCluster = KyoukoNetwork::m_config->getNumberOfNodes();

    uint32_t numberOfInitCluster = m_testClusterHandler->getNumberOfCluster();
    UNITTEST(numberOfInitCluster, 13)

    UNITTEST((int)m_testClusterHandler->getCluster(7)->getClusterType(), EDGE_CLUSTER)
    UNITTEST((int)m_testClusterHandler->getCluster(8)->getClusterType(), NODE_CLUSTER)

    metaData.push_back(m_testClusterHandler->getCluster(7)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(8)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(9)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(10)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(14)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(15)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(16)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(17)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(18)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(21)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(22)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(23)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(24)->getMetaData());

    ClusterMetaData totalData;
    totalData.positionOfDynamicBlocks = 0;
    totalData.positionOfStaticBlocks = 0;

    for(uint32_t i = 0; i < metaData.size(); i++)
    {
        totalData.numberOfStaticItems += metaData.at(i).numberOfStaticItems;
        totalData.numberOfDynamicItems += metaData.at(i).numberOfDynamicItems;

        totalData.numberOfStaticBlocks += metaData.at(i).numberOfStaticBlocks;
        totalData.numberOfDynamicBlocks += metaData.at(i).numberOfDynamicBlocks;

        totalData.positionOfDynamicBlocks += metaData.at(i).positionOfDynamicBlocks;
        totalData.positionOfStaticBlocks += metaData.at(i).positionOfStaticBlocks;
    }

    for(uint32_t i = 0; i < m_testClusterHandler->getNumberOfCluster(); i++)
    {
        Cluster* cluster = m_testClusterHandler->getClusterByIndex(i);

        for(uint32_t side = 0; side < 17; side++)
        {
            if(cluster->getNeighborId(side) != UNINIT_STATE_32)
            {
                ClusterID sourceId = cluster->getClusterId();
                ClusterID targetId = cluster->getNeighborId(side);
                ClusterID compareSource = m_testClusterHandler->getCluster(targetId)->getNeighborId(16 - side);
                UNITTEST(compareSource, sourceId);
            }
        }
    }

    UNITTEST((int)totalData.numberOfStaticItems, nodeNumberPerCluster*6)
    UNITTEST((int)totalData.numberOfDynamicItems, nodeNumberPerCluster*6)

    UNITTEST((int)totalData.numberOfStaticBlocks, 6)
    UNITTEST((int)totalData.numberOfDynamicBlocks, 7)

    UNITTEST((int)totalData.positionOfDynamicBlocks, 19)
    UNITTEST((int)totalData.positionOfStaticBlocks, 6)
}

/**
 * @brief InitTest::cleanupTestCase
 */
void InitTest::cleanupTestCase()
{
    delete m_networkInitializer;
    delete m_testClusterHandler;
}

}
