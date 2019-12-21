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
    : Kitsunemimi::Common::Test("IncomingBufferTest")
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
    TEST_EQUAL(testBuffer.addMessage(1), true);
    TEST_EQUAL(testBuffer.addMessage(1), true);
    TEST_EQUAL(testBuffer.addMessage(1), true);
    TEST_EQUAL(testBuffer.addMessage(1), true);
    TEST_EQUAL(testBuffer.addMessage(1), false);
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

    TEST_EQUAL(testBuffer.getMessage(), 1);
    testBuffer.reset();
    TEST_EQUAL(testBuffer.getMessage(), 2);
    TEST_EQUAL(testBuffer.getMessage(), 2);
    testBuffer.reset();
    TEST_EQUAL(testBuffer.getMessage(), 3);
}

/**
 * @brief isReady_test
 */
void
IncomingBufferTest::isReady_test()
{
    IncomingBuffer testBuffer;
    TEST_EQUAL(testBuffer.isReady(), false);
    testBuffer.addMessage(1);
    TEST_EQUAL(testBuffer.isReady(), true);
    TEST_EQUAL(testBuffer.getMessage(), 1);
    testBuffer.reset();
    TEST_EQUAL(testBuffer.isReady(), false);
    testBuffer.addMessage(1);
    testBuffer.addMessage(2);
    testBuffer.addMessage(3);
    TEST_EQUAL(testBuffer.isReady(), true);
}

} // namespace KyoukoMind
