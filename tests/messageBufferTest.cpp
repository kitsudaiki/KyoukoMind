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
    ClusterID tempIdIn = 42;
    ClusterID tempIdOut = 43;
    m_controller = new MessageController();
    //m_incomBuffer = new IncomingMessageBuffer(tempIdIn, m_controller);
    //m_ougoingBuffer = new OutgoingMessageBuffer(tempIdOut, m_controller);
}

/**
 * @brief MessageBufferTest::checkMessageBuffer
 */
void MessageBufferTest::checkMessageBuffer()
{
    ClusterID tempIdIn = 42;
    KyoChanEdge edge;
    edge.targetClusterPath = 42;
    edge.targetNodeId = 123;

    //UNITTEST(m_ougoingBuffer->addEdge(tempIdIn, 1, edge), true);
    //m_ougoingBuffer->sendFinishCycle(tempIdIn, 1);

    //std::vector<Message*>* testList = m_incomBuffer->getMessage(1);
    //UNITTEST(testList->size(), 1)
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
