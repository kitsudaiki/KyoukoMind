#include "initTest.h"

#include <core/clustering/clusterHandler.h>
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
 * @brief InitTest::cleanupTestCase
 */
void InitTest::cleanupTestCase()
{
    delete m_networkInitializer;
    delete m_testMessageController;
    delete m_testClusterHandler;
}

}
