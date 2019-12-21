/**
 *  @file    message_buffer_test.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "message_buffer_test.h"
#include <core/messaging/message_block_buffer.h>
#include <core/bricks/brick_objects/brick.h>

namespace KyoukoMind
{

MessageBufferTest::MessageBufferTest()
    : Kitsunemimi::Common::Test("MessageBufferTest")
{
    initTest();
    appandMessageBlock_test();
    addMessageBlock_test();
    getMessage_test();
    getNextMessage_test();
    finishMessage_test();
    clostTest();
}

/**
 * @brief initTest
 */
void
MessageBufferTest::initTest()
{
    m_network = new KyoukoMind::KyoukoNetwork();
    m_testBrick = new Brick(42, 0, 0);
    KyoukoNetwork::m_brickHandler->addBrick(42, m_testBrick);
}

/**
 * @brief appandMessageBlock_test
 */
void
MessageBufferTest::appandMessageBlock_test()
{
    // init
    MessageBlockBuffer buffer;

    DataMessage testMessage;
    testMessage.targetSide = 1;
    testMessage.targetBrickId = 42;

    // run test
    TEST_EQUAL(buffer.m_blocks.size(), 0);
    TEST_EQUAL(buffer.appandMessageBlock(&testMessage), 0);
    TEST_EQUAL(buffer.m_blocks[0]->containsMessages, 1);
    TEST_EQUAL(buffer.m_blocks.size(), 1);

    for(uint32_t i = 0; i < 1023; i++)
    {
        buffer.appandMessageBlock(&testMessage);
    }

    TEST_EQUAL(buffer.m_blocks.size(), 1);
    TEST_EQUAL(buffer.appandMessageBlock(&testMessage), 1024);
    TEST_EQUAL(buffer.m_blocks.size(), 2);

    testMessage.isLast = 1;
    TEST_EQUAL(buffer.appandMessageBlock(&testMessage), 1025);
    TEST_EQUAL(buffer.m_blocks[0]->containsMessages, 1024);
}

/**
 * @brief addMessageBlock_test
 */
void
MessageBufferTest::addMessageBlock_test()
{
    // init
    MessageBlockBuffer buffer;

    DataMessage testMessage;
    testMessage.targetSide = 1;
    testMessage.targetBrickId = 42;

    // run test
    TEST_EQUAL(buffer.m_blocks.size(), 0);
    buffer.addMessageBlock(0, (uint8_t*)&testMessage, 1);
    TEST_EQUAL(buffer.m_blocks[0]->containsMessages, 1);
    TEST_EQUAL(buffer.m_blocks.size(), 1);

    for(uint32_t i = 1; i < 1024; i++)
    {
        buffer.addMessageBlock(i, (uint8_t*)&testMessage, 1);
    }

    TEST_EQUAL(buffer.m_blocks.size(), 1);
    buffer.addMessageBlock(1024, (uint8_t*)&testMessage, 1);
    TEST_EQUAL(buffer.m_blocks.size(), 2);

    testMessage.isLast = 1;
    buffer.addMessageBlock(1025, (uint8_t*)&testMessage, 1);
    TEST_EQUAL(buffer.m_blocks[0]->containsMessages, 1024);
}

/**
 * @brief getMessage_test
 */
void
MessageBufferTest::getMessage_test()
{
    // init
    MessageBlockBuffer buffer;

    DataMessage testMessage;
    testMessage.targetSide = 1;
    testMessage.targetBrickId = 42;

    for(uint32_t i = 0; i < 1500; i++)
    {
        buffer.appandMessageBlock(&testMessage);
    }

    testMessage.type = 42;
    testMessage.isLast = 1;
    buffer.appandMessageBlock(&testMessage);

    // run test
    DataMessage* message = buffer.getMessage(1500);
    TEST_EQUAL(message->type, 42);
}

/**
 * @brief getNextMessage_test
 */
void
MessageBufferTest::getNextMessage_test()
{
    // init
    MessageBlockBuffer buffer;

    DataMessage testMessage;
    testMessage.targetSide = 1;
    testMessage.targetBrickId = 42;

    for(uint32_t i = 0; i < 1500; i++)
    {
        buffer.appandMessageBlock(&testMessage);
    }

    testMessage.type = 42;
    testMessage.isLast = 1;
    buffer.appandMessageBlock(&testMessage);

    testMessage.isLast = 0;
    testMessage.type = DATA_MESSAGE;
    for(uint32_t i = 0; i < 500; i++)
    {
        buffer.appandMessageBlock(&testMessage);
    }

    testMessage.type = 123;
    testMessage.isLast = 1;
    buffer.appandMessageBlock(&testMessage);

    testMessage.isLast = 0;
    testMessage.type = DATA_MESSAGE;

    // run test
    DataMessage* message = (DataMessage*)buffer.getNextMessage();
    TEST_EQUAL((uint32_t)message->type, 42);
    message = (DataMessage*)buffer.getNextMessage();
    TEST_EQUAL((uint32_t)message->type, 123);
}

/**
 * @brief finishMessage_test
 */
void
MessageBufferTest::finishMessage_test()
{
    // init
    MessageBlockBuffer buffer;

    DataMessage testMessage;
    testMessage.targetSide = 1;
    testMessage.targetBrickId = 42;

    for(uint32_t i = 0; i < 3000; i++)
    {
        buffer.appandMessageBlock(&testMessage);
    }

    // precheck
    TEST_EQUAL(buffer.m_blocks[0]->containsMessages, 1024);
    TEST_EQUAL(buffer.m_blocks[0]->processedMessages, 0);
    TEST_EQUAL(buffer.m_blocks.size(), 3);

    // run test
    buffer.finishMessage(0);
    TEST_EQUAL(buffer.m_blocks[0]->containsMessages, 1024);
    TEST_EQUAL(buffer.m_blocks[0]->processedMessages, 1);
    for(uint32_t i = 1; i < 2000; i++)
    {
        buffer.finishMessage(i);
    }
    TEST_EQUAL(buffer.m_blocks.size(), 2);
}

/**
 * @brief clostTest
 */
void
MessageBufferTest::clostTest()
{
    delete m_network;
}

} // namespace KyoukoMind
