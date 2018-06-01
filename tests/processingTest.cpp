/**
 *  @file    processingTest.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "processingTest.h"
#include <core/messaging/messageController.h>
#include <core/messaging/messageQueues/messageBuffer.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

#include <core/clustering/cluster/nodeCluster.h>
#include <core/clustering/cluster/edgeCluster.h>

#include <core/processing/processingThreads/cpu/nextChooser.h>
#include <core/processing/processingThreads/cpu/edgeClusterProcessing.h>

namespace KyoukoMind
{

/**
 * @brief ProcessingTest::ProcessingTest
 */
ProcessingTest::ProcessingTest() : CommonTest("ProcessingTest")
{
    initTestCase();
    checkProcessing();
    cleanupTestCase();
}

/**
 * @brief ProcessingTest::initTestCase
 */
void ProcessingTest::initTestCase()
{
    m_controller = new MessageController();
    m_nextChooser = new NextChooser();
    //m_nodeProcessing = new ClusterProcessing(m_nextChooser);

    NodeCluster* initCluster = new NodeCluster(1337, "/tmp/test", 42);

    Neighbor neighbor;
    neighbor.targetClusterId = 1;
    neighbor.neighborType = NODE_CLUSTER;
    initCluster->addNeighbor(15, neighbor);
    initCluster->initMessageBuffer(m_controller);

    m_incomBuffer = initCluster->getIncomingMessageBuffer();
    m_ougoingBuffer = initCluster->getOutgoingMessageBuffer();

    m_nodeCluster1 = new NodeCluster(1, "/tmp/test", 1);
    m_nodeCluster2 = new NodeCluster(2, "/tmp/test", 1);
    m_nodeCluster3 = new NodeCluster(3, "/tmp/test", 1);

    Neighbor neighbor1;
    neighbor1.targetClusterId = 2;
    neighbor1.neighborType = NODE_CLUSTER;
    m_nodeCluster1->addNeighbor(3, neighbor1);
    m_nodeCluster1->initMessageBuffer(m_controller);
    m_nodeCluster1->getNodeBlock()[0].targetClusterPath = 3;
    m_nodeCluster1->getNodeBlock()[0].targetAxonId = 0;

    Neighbor neighbor2;
    neighbor2.targetClusterId = 3;
    neighbor2.neighborType = NODE_CLUSTER;
    m_nodeCluster2->addNeighbor(3, neighbor2);
    m_nodeCluster2->initMessageBuffer(m_controller);

    Neighbor neighbor3;
    neighbor3.targetClusterId = 2;
    neighbor3.neighborType = NODE_CLUSTER;
    m_nodeCluster3->addNeighbor(3, neighbor3);
    m_nodeCluster3->initMessageBuffer(m_controller);
}

/**
 * @brief ProcessingTest::checkProcessing
 */
void ProcessingTest::checkProcessing()
{
    KyoChanDirectEdgeContainer edge;
    edge.targetNodeId = 0;
    edge.weight = 100.0;

    UNITTEST(m_ougoingBuffer->addDirectEdge(15, &edge), true);
    m_ougoingBuffer->finishCycle(15, 0);

    //m_nodeProcessing->processMessagesEdges(m_nodeCluster1);
    //m_nodeProcessing->processNodes(m_nodeCluster1);
    m_nodeCluster1->finishCycle(1);
    //m_nodeProcessing->processMessagesEdges(m_nodeCluster2);
    //m_nodeProcessing->processNodes(m_nodeCluster2);
    m_nodeCluster2->finishCycle(1);
    //m_nodeProcessing->processMessagesEdges(m_nodeCluster3);
    //m_nodeProcessing->processNodes(m_nodeCluster3);
    m_nodeCluster3->finishCycle(1);
}

/**
 * @brief ProcessingTest::cleanupTestCase
 */
void ProcessingTest::cleanupTestCase()
{
    delete m_nodeCluster1;
    delete m_nodeCluster2;
    delete m_nodeCluster3;
}

}
