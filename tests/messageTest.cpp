/**
 *  @file    messageTest.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "messageTest.h"

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messages/replyMessage.h>

namespace KyoukoMind
{

MessageTest::MessageTest() : CommonTest("MessageTest")
{
    initTestCase();
    checkDataMessage();
    checkReplyMessage();
    cleanupTestCase();
}

void MessageTest::initTestCase()
{

}

void MessageTest::checkDataMessage()
{
    DataMessage message(1, 3, 4);

    CommonMessageData metaData = message.getMetaData();

    UNITTEST(metaData.requiredReply, 1);
    UNITTEST((uint32_t)metaData.targetSite, 4);
    UNITTEST(metaData.type, DATA_MESSAGE);

    KyoChanEdgeContainer testEdge;
    testEdge.targetClusterPath = 1;
    testEdge.targetNodeId = 2;
    testEdge.weight = 3;
    message.addEdge(&testEdge);

    KyoChanEdgeContainer testEdge2;
    testEdge2.targetClusterPath = 4;
    testEdge2.targetNodeId = 5;
    testEdge2.weight = 6;
    message.addEdge(&testEdge2);

    void* data = message.getData();
    uint32_t size = message.getDataSize();

    DataMessage newMessage(data, size);

    CommonMessageData metaData2 = newMessage.getMetaData();

    UNITTEST(metaData2.requiredReply, 1);
    UNITTEST((uint32_t)metaData2.targetSite, 4);
    UNITTEST(metaData2.type, DATA_MESSAGE);

    UNITTEST((int)newMessage.getNumberOfEntries(), 2);

    uint8_t* newData = (uint8_t*)newMessage.getPayload();

    UNITTEST(newData[0], EDGE_CONTAINER);

    KyoChanEdgeContainer* edges = (KyoChanEdgeContainer*)newData;

    UNITTEST(edges[0].targetClusterPath, 1);
    UNITTEST(edges[0].targetNodeId, 2);
    UNITTEST(edges[0].weight, 3);
}

void MessageTest::checkReplyMessage()
{
    ReplyMessage message(1, 3);

    CommonMessageData metaData = message.getMetaData();

    UNITTEST(metaData.requiredReply, 0);
    UNITTEST((uint32_t)metaData.targetSite, 3);
    UNITTEST(metaData.type, REPLY_MESSAGE);
}

void MessageTest::cleanupTestCase()
{

}

}
