/**
 *  @file    messageBufferTest.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef MESSAGEBUFFERTEST_H
#define MESSAGEBUFFERTEST_H

#include <common.h>
#include <tests/commonTest.h>

namespace KyoukoMind
{
class MessageController;
class IncomingMessageBuffer;
class MessageBuffer;
class OutgoingMessageBuffer;
class NodeCluster;

class MessageBufferTest : public CommonTest
{

public:
    MessageBufferTest();

private:
    void initTestCase();
    void checkMessageBuffer();
    void cleanupTestCase();

    NodeCluster* m_fakeCluster = nullptr;
    MessageController* m_controller = nullptr;
    IncomingMessageBuffer* m_incomBuffer= nullptr;
    OutgoingMessageBuffer* m_ougoingBuffer= nullptr;
};

}

#endif // MESSAGEBUFFERTEST_H
