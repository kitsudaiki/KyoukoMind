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
    : Kitsune::Common::Test("MessageBufferTest")
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
    UNITTEST(buffer.m_blocks.size(), 0);
    UNITTEST(buffer.appandMessageBlock(&testMessage), 0);
    UNITTEST(buffer.m_blocks[0]->containsMessages, 1);
    UNITTEST(buffer.m_blocks.size(), 1);

    for(uint32_t i = 0; i < 1023; i++)
    {
        buffer.appandMessageBlock(&testMessage);
    }

    UNITTEST(buffer.m_blocks.size(), 1);
    UNITTEST(buffer.appandMessageBlock(&testMessage), 1024);
    UNITTEST(buffer.m_blocks.size(), 2);

    testMessage.isLast = 1;
    UNITTEST(buffer.appandMessageBlock(&testMessage), 1025);
    UNITTEST(buffer.m_blocks[0]->containsMessages, 1024);
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
    UNITTEST(buffer.m_blocks.size(), 0);
    buffer.addMessageBlock(0, (uint8_t*)&testMessage, 1);
    UNITTEST(buffer.m_blocks[0]->containsMessages, 1);
    UNITTEST(buffer.m_blocks.size(), 1);

    for(uint32_t i = 1; i < 1024; i++)
    {
        buffer.addMessageBlock(i, (uint8_t*)&testMessage, 1);
    }

    UNITTEST(buffer.m_blocks.size(), 1);
    buffer.addMessageBlock(1024, (uint8_t*)&testMessage, 1);
    UNITTEST(buffer.m_blocks.size(), 2);

    testMessage.isLast = 1;
    buffer.addMessageBlock(1025, (uint8_t*)&testMessage, 1);
    UNITTEST(buffer.m_blocks[0]->containsMessages, 1024);
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
    UNITTEST(message->type, 42);
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
    UNITTEST((uint32_t)message->type, 42);
    message = (DataMessage*)buffer.getNextMessage();
    UNITTEST((uint32_t)message->type, 123);
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
    UNITTEST(buffer.m_blocks[0]->containsMessages, 1024);
    UNITTEST(buffer.m_blocks[0]->processedMessages, 0);
    UNITTEST(buffer.m_blocks.size(), 3);

    // run test
    buffer.finishMessage(0);
    UNITTEST(buffer.m_blocks[0]->containsMessages, 1024);
    UNITTEST(buffer.m_blocks[0]->processedMessages, 1);
    for(uint32_t i = 1; i < 2000; i++)
    {
        buffer.finishMessage(i);
    }
    UNITTEST(buffer.m_blocks.size(), 2);
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
