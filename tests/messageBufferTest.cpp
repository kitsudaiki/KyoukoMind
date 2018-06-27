/**
 *  @file    messageBufferTest.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "messageBufferTest.h"
#include <core/messaging/messageController.h>

#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/messaging/messageQueues/messageBuffer.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

namespace KyoukoMind
{

/**
 * @brief MessageBufferTest::MessageBufferTest
 */
MessageBufferTest::MessageBufferTest() : CommonTest("MessageBufferTest")
{
    initTestCase();
    checkMessageBuffer();
    cleanupTestCase();
}

/**
 * @brief MessageBufferTest::initTestCase
 */
void MessageBufferTest::initTestCase()
{
    m_controller = new MessageController();
    m_fakeCluster = new NodeCluster(1337, "/tmp/test");

    Neighbor neighbor;
    neighbor.targetClusterId = 1337;
    neighbor.neighborType = NODE_CLUSTER;

    m_fakeCluster->addNeighbor(0, neighbor);
    m_fakeCluster->addNeighbor(1, neighbor);
    m_fakeCluster->addNeighbor(2, neighbor);
    m_fakeCluster->addNeighbor(3, neighbor);
    m_fakeCluster->addNeighbor(4, neighbor);
    m_fakeCluster->addNeighbor(5, neighbor);
    m_fakeCluster->addNeighbor(6, neighbor);
    m_fakeCluster->addNeighbor(7, neighbor);
    m_fakeCluster->addNeighbor(8, neighbor);
    m_fakeCluster->addNeighbor(9, neighbor);
    m_fakeCluster->addNeighbor(10, neighbor);
    m_fakeCluster->addNeighbor(11, neighbor);
    m_fakeCluster->addNeighbor(12, neighbor);
    m_fakeCluster->addNeighbor(13, neighbor);
    m_fakeCluster->addNeighbor(14, neighbor);
    m_fakeCluster->addNeighbor(15, neighbor);
    m_fakeCluster->addNeighbor(16, neighbor);
    m_fakeCluster->initMessageBuffer(m_controller);

    m_incomBuffer = m_fakeCluster->getIncomingMessageBuffer();
    m_ougoingBuffer = m_fakeCluster->getOutgoingMessageBuffer();
}

/**
 * @brief MessageBufferTest::checkMessageBuffer
 */
void MessageBufferTest::checkMessageBuffer()
{
    KyoChanForwardEdgeContainer edge;
    edge.targetEdgeSectionId = 1;

    UNITTEST(m_ougoingBuffer->addForwardEdge(0, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(1, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(2, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(3, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(4, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(5, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(6, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(7, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(8, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(9, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(10, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(11, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(12, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(13, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(14, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(15, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(16, &edge), true);
    UNITTEST(m_ougoingBuffer->addForwardEdge(17, &edge), false);

    m_ougoingBuffer->finishCycle(0, 0);
    m_ougoingBuffer->finishCycle(1, 0);
    m_ougoingBuffer->finishCycle(2, 0);
    m_ougoingBuffer->finishCycle(3, 0);
    m_ougoingBuffer->finishCycle(4, 0);
    m_ougoingBuffer->finishCycle(5, 0);
    m_ougoingBuffer->finishCycle(6, 0);
    m_ougoingBuffer->finishCycle(7, 0);
    m_ougoingBuffer->finishCycle(8, 0);
    m_ougoingBuffer->finishCycle(9, 0);
    m_ougoingBuffer->finishCycle(10, 0);
    m_ougoingBuffer->finishCycle(11, 0);
    m_ougoingBuffer->finishCycle(12, 0);
    m_ougoingBuffer->finishCycle(13, 0);
    m_ougoingBuffer->finishCycle(14, 0);
    m_ougoingBuffer->finishCycle(15, 0);
    m_ougoingBuffer->finishCycle(16, 0);

    UNITTEST(m_incomBuffer->getMessage(0)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(1)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(2)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(3)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(4)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(5)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(6)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(7)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(8)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(9)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(10)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(11)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(12)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(13)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(14)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(15)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(m_incomBuffer->getMessage(16)->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
}

/**
 * @brief MessageBufferTest::cleanupTestCase
 */
void MessageBufferTest::cleanupTestCase()
{
    delete m_incomBuffer;
    delete m_ougoingBuffer;
    delete m_controller;
}

}
