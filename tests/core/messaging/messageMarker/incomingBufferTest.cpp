#include "incomingBufferTest.h"
#include <core/messaging/messageMarker/incomingBuffer.h>

namespace KyoukoMind
{


IncomingBufferTest::IncomingBufferTest()
    : Kitsune::CommonTest("IncomingBufferTest")
{
    testAddMessageAndFinish();
    testGetMessage();
    testIsReady();
}

void IncomingBufferTest::testAddMessageAndFinish()
{
    IncomingBuffer testBuffer;
    UNITTEST(testBuffer.addMessage(1), true);
    UNITTEST(testBuffer.addMessage(1), true);
    UNITTEST(testBuffer.addMessage(1), true);
    UNITTEST(testBuffer.addMessage(1), true);
    UNITTEST(testBuffer.addMessage(1), false);
}

void IncomingBufferTest::testGetMessage()
{
    IncomingBuffer testBuffer;
    testBuffer.addMessage(1);
    testBuffer.addMessage(2);
    testBuffer.addMessage(3);

    UNITTEST(testBuffer.getMessage(), 1);
    testBuffer.reset();
    UNITTEST(testBuffer.getMessage(), 2);
    UNITTEST(testBuffer.getMessage(), 2);
    testBuffer.reset();
    UNITTEST(testBuffer.getMessage(), 3);
}

void IncomingBufferTest::testIsReady()
{
    IncomingBuffer testBuffer;
    UNITTEST(testBuffer.isReady(), false);
    testBuffer.addMessage(1);
    UNITTEST(testBuffer.isReady(), true);
    UNITTEST(testBuffer.getMessage(), 1);
    testBuffer.reset();
    UNITTEST(testBuffer.isReady(), false);
    testBuffer.addMessage(1);
    testBuffer.addMessage(2);
    testBuffer.addMessage(3);
    UNITTEST(testBuffer.isReady(), true);
}

}
