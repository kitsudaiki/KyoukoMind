/**
 *  @file    messageBufferTest.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "messageBufferTest.h"

#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <messageQueues/incomingMessageBuffer.h>
#include <messageQueues/outgoingMessageBuffer.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{

/**
 * @brief MessageBufferTest::MessageBufferTest
 */
MessageBufferTest::MessageBufferTest() : Kitsune::CommonTest("MessageBufferTest")
{
    initTestCase();
    checkInitializing();
    checkMessageBuffer();
    cleanupTestCase();
}

/**
 * @brief MessageBufferTest::initTestCase
 */
void MessageBufferTest::initTestCase()
{
    m_fakeCluster = new NodeCluster(1337, "/tmp/test");
}

/**
 * @brief MessageBufferTest::checkInitializing
 */
void MessageBufferTest::checkInitializing()
{
    UNITTEST(m_fakeCluster->setNeighbor(0, 1337), true);
    UNITTEST(m_fakeCluster->setNeighbor(16, 1337), true);
    UNITTEST(m_fakeCluster->setNeighbor(17, 1337), false);


    UNITTEST(m_fakeCluster->setNewConnection(0, m_fakeCluster->getIncomingMessageBuffer(16)), true);
    UNITTEST(m_fakeCluster->setNewConnection(16, m_fakeCluster->getIncomingMessageBuffer(0)), true);
    UNITTEST(m_fakeCluster->setNewConnection(17, m_fakeCluster->getIncomingMessageBuffer(0)), false);
}

/**
 * @brief MessageBufferTest::checkMessageBuffer
 */
void MessageBufferTest::checkMessageBuffer()
{
    KyoChanForwardEdgeContainer edge;

    // get
    UNITTEST(m_fakeCluster->isBufferReady(), true);
    UNITTEST(m_fakeCluster->getIncomingMessageBuffer(0)->getMessage()->getPayloadSize(), 0)
    UNITTEST(m_fakeCluster->getIncomingMessageBuffer(16)->getMessage()->getPayloadSize(), 0)

    m_fakeCluster->finishCycle(0);
    UNITTEST(m_fakeCluster->isBufferReady(), true);

    // get
    m_fakeCluster->getIncomingMessageBuffer(0)->getMessage();
    m_fakeCluster->getIncomingMessageBuffer(16)->getMessage();

    edge.targetEdgeSectionId = 1;
    // set
    UNITTEST(m_fakeCluster->getOutgoingMessageBuffer(0)->addData(&edge), true);
    UNITTEST(m_fakeCluster->getOutgoingMessageBuffer(16)->addData(&edge), true);
    UNITTEST(m_fakeCluster->getOutgoingMessageBuffer(17)->addData(&edge), false);

    m_fakeCluster->finishCycle(0);
    UNITTEST(m_fakeCluster->isBufferReady(), true);

    // get
    Kitsune::MindMessaging::DataMessage* message0 = m_fakeCluster->getIncomingMessageBuffer(0)->getMessage();
    Kitsune::MindMessaging::DataMessage* message16 = m_fakeCluster->getIncomingMessageBuffer(16)->getMessage();
    UNITTEST(message0->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(message16->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))

    m_fakeCluster->finishCycle(0);
    UNITTEST(m_fakeCluster->isBufferReady(), true);

    KyoChanAxonEdgeContainer edge2;
    edge2.targetAxonId = 1;

    // get
    m_fakeCluster->getIncomingMessageBuffer(0)->getMessage();
    m_fakeCluster->getIncomingMessageBuffer(16)->getMessage();

    // set
    UNITTEST(m_fakeCluster->getOutgoingMessageBuffer(0)->addData(&edge2), true);
    UNITTEST(m_fakeCluster->getOutgoingMessageBuffer(16)->addData(&edge2), true);
    UNITTEST(m_fakeCluster->getOutgoingMessageBuffer(17)->addData(&edge2), false);

    m_fakeCluster->finishCycle(0);
    UNITTEST(m_fakeCluster->isBufferReady(), true);

    // get
    UNITTEST(m_fakeCluster->getIncomingMessageBuffer(0)->getMessage()->getPayloadSize(), sizeof(KyoChanAxonEdgeContainer))
    UNITTEST(m_fakeCluster->getIncomingMessageBuffer(16)->getMessage()->getPayloadSize(), sizeof(KyoChanAxonEdgeContainer))


    m_fakeCluster->finishCycle(0);
    UNITTEST(m_fakeCluster->isBufferReady(), true);

    UNITTEST(m_fakeCluster->getIncomingMessageBuffer(0)->getMessage()->getPayloadSize(), 0)
    UNITTEST(m_fakeCluster->getIncomingMessageBuffer(16)->getMessage()->getPayloadSize(), 0)

}

/**
 * @brief MessageBufferTest::cleanupTestCase
 */
void MessageBufferTest::cleanupTestCase()
{
}

}
