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
    EdgeCluster* fakeCluster = new NodeCluster(1337, "/tmp/test", 42);

    Neighbor neighbor;
    neighbor.targetClusterId = 1337;
    neighbor.neighborType = NODE_CLUSTER;
    fakeCluster->addNeighbor(15, neighbor);

    fakeCluster->initMessageBuffer(m_controller);

    m_incomBuffer = fakeCluster->getIncomingMessageBuffer();
    m_ougoingBuffer = fakeCluster->getOutgoingMessageBuffer();
}

/**
 * @brief MessageBufferTest::checkMessageBuffer
 */
void MessageBufferTest::checkMessageBuffer()
{
    KyoChanForwardEdgeContainer edge;
    edge.targetEdgeSectionId = 1;

    UNITTEST(m_ougoingBuffer->addForwardEdge(15, &edge), true);
    m_ougoingBuffer->finishCycle(15);

    Message* message = m_incomBuffer->getMessage(0);
    UNITTEST(message->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
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
