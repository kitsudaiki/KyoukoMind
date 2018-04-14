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
#include <core/messaging/messages/learningMessage.h>
#include <core/messaging/messages/learningReplyMessage.h>

namespace KyoukoMind
{

MessageTest::MessageTest() : CommonTest("MessageTest")
{
    initTestCase();
    checkDataMessage();
    checkReplyMessage();
    checkLerningMessage();
    checkLearingReplyMessage();
    cleanupTestCase();
}

void MessageTest::initTestCase()
{

}

void MessageTest::checkDataMessage()
{
    DataMessage message(1, 2, 3, 4);

    CommonMessageData metaData = message.getMetaData();

    UNITTEST(metaData.messageId, 12884901890);
    UNITTEST(metaData.requiredReply, 1);
    UNITTEST((uint32_t)metaData.targetSite, 4);
    UNITTEST(metaData.type, DATA_MESSAGE);

    KyoChanEdge testEdge;
    testEdge.targetClusterPath = 1;
    testEdge.targetNodeId = 2;
    testEdge.weight = 3;

    KyoChanEdge testEdge2;
    testEdge2.targetClusterPath = 4;
    testEdge2.targetNodeId = 5;
    testEdge2.weight = 6;

    UNITTEST(message.addEdge(testEdge), true);
    UNITTEST(message.addEdge(testEdge2), true);

    uint8_t* data = message.convertToByteArray();

    DataMessage newMessage;
    newMessage.convertFromByteArray(data);

    CommonMessageData metaData2 = newMessage.getMetaData();

    UNITTEST(metaData2.messageId, 12884901890);
    UNITTEST(metaData2.requiredReply, 1);
    UNITTEST((uint32_t)metaData2.targetSite, 4);
    UNITTEST(metaData2.type, DATA_MESSAGE);

    UNITTEST(newMessage.getNumberOfEdges(), 2);
    KyoChanEdge* edges = newMessage.getEdges();

    UNITTEST(edges[0].targetClusterPath, 1);
    UNITTEST(edges[0].targetNodeId, 2);
    UNITTEST(edges[0].weight, 3);
}

void MessageTest::checkReplyMessage()
{
    ReplyMessage message(1, 12884901890, 3);

    CommonMessageData metaData = message.getMetaData();

    UNITTEST(metaData.messageId, 12884901890);
    UNITTEST(metaData.requiredReply, 0);
    UNITTEST((uint32_t)metaData.targetSite, 3);
    UNITTEST(metaData.type, REPLY_MESSAGE);
}

void MessageTest::checkLerningMessage()
{
    LearningMessage message(1, 2, 3, 4);

    CommonMessageData metaData = message.getMetaData();

    UNITTEST(metaData.messageId, 12884901890);
    UNITTEST(metaData.requiredReply, 1);
    UNITTEST((uint32_t)metaData.targetSite, 4);
    UNITTEST(metaData.type, LEARNING_MESSAGE);

    KyoChanNewEdge testEdge;
    testEdge.weight = 1;
    testEdge.newEdgeId = 2;
    testEdge.sourceClusterId = 3;
    testEdge.sourceNodeId = 4;

    KyoChanNewEdge testEdge2;
    testEdge2.weight = 5;
    testEdge2.newEdgeId = 6;
    testEdge2.sourceClusterId = 7;
    testEdge2.sourceNodeId = 8;

    UNITTEST(message.addNewEdge(testEdge), true);
    UNITTEST(message.addNewEdge(testEdge2), true);

    uint8_t* data = message.convertToByteArray();

    LearningMessage newMessage;
    newMessage.convertFromByteArray(data);

    CommonMessageData metaData2 = newMessage.getMetaData();

    UNITTEST(metaData2.messageId, 12884901890);
    UNITTEST(metaData2.requiredReply, 1);
    UNITTEST((uint32_t)metaData2.targetSite, 4);
    UNITTEST(metaData2.type, LEARNING_MESSAGE);

    UNITTEST(newMessage.getNumberOfNewEdges(), 2);
    KyoChanNewEdge* edges = newMessage.getNewEdges();

    UNITTEST(edges[0].weight, 1);
    UNITTEST(edges[0].newEdgeId, 2);
    UNITTEST(edges[0].sourceClusterId, 3);
    UNITTEST(edges[0].sourceNodeId, 4);
}

void MessageTest::checkLearingReplyMessage()
{
    LearningReplyMessage message(1, 12884901890, 3);

    CommonMessageData metaData = message.getMetaData();

    UNITTEST(metaData.messageId, 12884901890);
    UNITTEST(metaData.requiredReply, 1);
    UNITTEST((uint32_t)metaData.targetSite, 3);
    UNITTEST(metaData.type, LEARNING_REPLY_MESSAGE);

    KyoChanNewEdgeReply testEdge;
    testEdge.newEdgeId = 1;
    testEdge.targetClusterId = 2;
    testEdge.targetNodeId = 3;
    testEdge.sourceClusterId = 4;
    testEdge.sourceNodeId = 5;

    KyoChanNewEdgeReply testEdge2;
    testEdge2.newEdgeId = 6;
    testEdge2.targetClusterId = 7;
    testEdge2.targetNodeId = 8;
    testEdge2.sourceClusterId = 9;
    testEdge2.sourceNodeId = 10;

    UNITTEST(message.addNewEdgeReply(testEdge), true);
    UNITTEST(message.addNewEdgeReply(testEdge2), true);

    uint8_t* data = message.convertToByteArray();

    LearningReplyMessage newMessage;
    newMessage.convertFromByteArray(data);

    CommonMessageData metaData2 = newMessage.getMetaData();

    UNITTEST(metaData2.messageId, 12884901890);
    UNITTEST(metaData2.requiredReply, 1);
    UNITTEST((uint32_t)metaData2.targetSite, 3);
    UNITTEST(metaData2.type, LEARNING_REPLY_MESSAGE);

    UNITTEST(newMessage.getNumberOfEdgeReplys(), 2);
    KyoChanNewEdgeReply* edges = newMessage.getNewEdgeReplys();

    UNITTEST(edges[0].newEdgeId, 1);
    UNITTEST(edges[0].targetClusterId, 2);
    UNITTEST(edges[0].targetNodeId, 3);
    UNITTEST(edges[0].sourceClusterId, 4);
    UNITTEST(edges[0].sourceNodeId, 5);
}

void MessageTest::cleanupTestCase()
{

}

}
