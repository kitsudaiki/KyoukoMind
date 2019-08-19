/**
 *  @file    incoming_buffer_test.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "incoming_buffer_test.h"
#include <core/messaging/message_buffer/incoming_buffer.h>

namespace KyoukoMind
{


IncomingBufferTest::IncomingBufferTest()
    : Kitsune::Common::Test("IncomingBufferTest")
{
    addMessageAndFinish_test();
    getMessage_test();
    isReady_test();
}

/**
 * @brief addMessageAndFinish_test
 */
void
IncomingBufferTest::addMessageAndFinish_test()
{
    IncomingBuffer testBuffer;
    UNITTEST(testBuffer.addMessage(1), true);
    UNITTEST(testBuffer.addMessage(1), true);
    UNITTEST(testBuffer.addMessage(1), true);
    UNITTEST(testBuffer.addMessage(1), true);
    UNITTEST(testBuffer.addMessage(1), false);
}

/**
 * @brief getMessage_test
 */
void
IncomingBufferTest::getMessage_test()
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

/**
 * @brief isReady_test
 */
void
IncomingBufferTest::isReady_test()
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

} // namespace KyoukoMind
