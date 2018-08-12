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
#include <commonTest.h>

namespace Networking
{
class IncomingMessageBuffer;
class OutgoingMessageBuffer;
}

namespace KyoukoMind
{
class NodeCluster;

class MessageBufferTest : public Kitsune::CommonTest
{

public:
    MessageBufferTest();

private:
    void initTestCase();
    void checkInitializing();
    void checkMessageBuffer();
    void cleanupTestCase();

    NodeCluster* m_fakeCluster = nullptr;
};

}

#endif // MESSAGEBUFFERTEST_H
