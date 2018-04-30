#include "processingtest.h"
#include <core/messaging/messageController.h>
#include <core/messaging/messageQueues/messageBuffer.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

#include <core/clustering/cluster/nodeCluster.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/emptyCluster.h>

#include <core/processing/processingThreads/cpu/nextChooser.h>
#include <core/processing/processingThreads/cpu/axonprocessing.h>
#include <core/processing/processingThreads/cpu/nodeprocessing.h>
#include <core/processing/processingThreads/cpu/edgeprocessing.h>

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
    m_axonProcessing = new AxonProcessing(m_nextChooser);
    m_nodeProcessing = new NodeProcessing();
    m_edgeProcessing = new EdgeProcessing(m_nextChooser);

    Cluster* initCluster = new Cluster(1337, NODE_CLUSTER, "/tmp/test");

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
    m_nodeCluster2->initAxonBlocks(1);

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
    KyoChanEdgeForewardContainer edge;
    edge.targetClusterPath = 0;
    edge.targetNodeId = 0;
    edge.weight = 100.0;

    UNITTEST(m_ougoingBuffer->addEdge(15, &edge), true);
    m_ougoingBuffer->finishCycle(15);

    OUTPUT("==========================================================")
    m_edgeProcessing->processInputMessages(m_nodeCluster1);
    m_edgeProcessing->processIncomingMessages((EdgeCluster*)m_nodeCluster1);
    m_nodeProcessing->processNodes(m_nodeCluster1);
    m_axonProcessing->processAxons((EdgeCluster*)m_nodeCluster1);
    m_nodeCluster1->getPendingEdges()->checkPendingEdges();
    m_nodeCluster1->finishCycle();
    OUTPUT("==========================================================")
    m_edgeProcessing->processInputMessages(m_nodeCluster2);
    m_edgeProcessing->processIncomingMessages((EdgeCluster*)m_nodeCluster2);
    m_nodeProcessing->processNodes(m_nodeCluster2);
    m_axonProcessing->processAxons((EdgeCluster*)m_nodeCluster2);
    m_nodeCluster2->getPendingEdges()->checkPendingEdges();
    m_nodeCluster2->finishCycle();
    OUTPUT("==========================================================")
    m_edgeProcessing->processInputMessages(m_nodeCluster3);
    m_edgeProcessing->processIncomingMessages((EdgeCluster*)m_nodeCluster3);
    m_nodeProcessing->processNodes(m_nodeCluster3);
    m_axonProcessing->processAxons((EdgeCluster*)m_nodeCluster3);
    m_nodeCluster3->getPendingEdges()->checkPendingEdges();
    m_nodeCluster3->finishCycle();
    OUTPUT("==========================================================")
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
