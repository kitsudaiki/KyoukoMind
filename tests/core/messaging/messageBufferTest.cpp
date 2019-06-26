/**
 *  @file    messageBufferTest.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "messageBufferTest.h"
#include <core/messaging/messageBlockBuffer.h>
#include <core/bricks/brickObjects/brick.h>

namespace KyoukoMind
{

MessageBufferTest::MessageBufferTest()
    : Kitsune::CommonTest("MessageBufferTest")
{
    initTest();
    testAppandMessageBlock();
    testAddMessageBlock();
    testGetMessage();
    testGetNextMessage();
    testFinishMessage();
    clostTest();
}

void MessageBufferTest::initTest()
{
    m_network = new KyoukoMind::KyoukoNetwork();
    m_testBrick = new Brick(42, 0, 0);
    KyoukoNetwork::m_brickHandler->addBrick(42, m_testBrick);
}

void MessageBufferTest::testAppandMessageBlock()
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

void MessageBufferTest::testAddMessageBlock()
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

void MessageBufferTest::testGetMessage()
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

void MessageBufferTest::testGetNextMessage()
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

void MessageBufferTest::testFinishMessage()
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

void MessageBufferTest::clostTest()
{
    delete m_network;
}

}
