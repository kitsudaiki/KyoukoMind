/**
 *  @file    initTestcpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "initTest.h"
#include <kyochanNetwork.h>

#include <core/clustering/clusterHandler.h>
#include <core/clustering/clusterQueue.h>

#include <core/messaging/messageController.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/initializing/axonInitializer.h>
#include <core/initializing/clusterInitilizer.h>
#include <core/initializing/networkInitializer.h>

namespace KyoukoMind
{

/**
 * @brief InitTest::InitTest
 */
InitTest::InitTest() : CommonTest("InitTest")
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
    m_testMessageController = new MessageController();
    m_networkInitializer = new NetworkInitializer(m_testClusterContent,
                                                  "/tmp/",
                                                  m_testClusterHandler,
                                                  m_testMessageController);
    m_networkInitializer->initNetwork();
}

/**
 * @brief InitTest::checkInit
 */
void InitTest::checkInit()
{
    std::vector<ClusterMetaData> metaData;
    bool ok = false;
    uint32_t nodeNumberPerCluster = KyoukoNetwork::m_config->getNumberOfNodes(&ok);

    uint32_t numberOfInitCluster = m_testClusterHandler->getNumberOfCluster();
    UNITTEST(numberOfInitCluster, 13)

    UNITTEST((int)m_testClusterHandler->getCluster(7)->getClusterType(), EDGE_CLUSTER)
    UNITTEST((int)m_testClusterHandler->getCluster(8)->getClusterType(), NODE_CLUSTER)

    metaData.push_back(m_testClusterHandler->getCluster(7)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(8)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(12)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(13)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(14)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(17)->getMetaData());
    metaData.push_back(m_testClusterHandler->getCluster(18)->getMetaData());

    ClusterMetaData totalData;
    totalData.positionForwardEdgeBlocks = 0;
    totalData.positionNodeBlocks = 0;
    totalData.positionOfEdgeBlock = 0;

    for(uint32_t i = 0; i < metaData.size(); i++)
    {
        totalData.numberOfNodes += metaData.at(i).numberOfNodes;
        totalData.numberOfForwardEdgeSections += metaData.at(i).numberOfForwardEdgeSections;
        totalData.numberOfPendingForwardEdgeSections += metaData.at(i).numberOfPendingForwardEdgeSections;

        totalData.numberOfNodeBlocks += metaData.at(i).numberOfNodeBlocks;
        totalData.numberOfEdgeBlocks += metaData.at(i).numberOfEdgeBlocks;
        totalData.numberOfForwardEdgeBlocks += metaData.at(i).numberOfForwardEdgeBlocks;

        totalData.positionForwardEdgeBlocks += metaData.at(i).positionForwardEdgeBlocks;
        totalData.positionNodeBlocks += metaData.at(i).positionNodeBlocks;
        totalData.positionOfEdgeBlock += metaData.at(i).positionOfEdgeBlock;
    }

    UNITTEST((int)totalData.numberOfNodes, nodeNumberPerCluster*6)
    UNITTEST((int)totalData.numberOfForwardEdgeSections, nodeNumberPerCluster*6)
    UNITTEST((int)totalData.numberOfPendingForwardEdgeSections, 0)

    UNITTEST((int)totalData.numberOfNodeBlocks, 1*6)
    UNITTEST((int)totalData.numberOfEdgeBlocks, 0)
    UNITTEST((int)totalData.numberOfForwardEdgeBlocks, 1*7)

    UNITTEST((int)totalData.positionForwardEdgeBlocks, 7*1)
    UNITTEST((int)totalData.positionNodeBlocks, 7*1)
    UNITTEST((int)totalData.positionOfEdgeBlock, 7+6)
}

/**
 * @brief InitTest::cleanupTestCase
 */
void InitTest::cleanupTestCase()
{
    delete m_networkInitializer;
    delete m_testMessageController;
    delete m_testClusterHandler;
}

}
