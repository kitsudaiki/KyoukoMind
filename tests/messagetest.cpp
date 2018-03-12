#include "messagetest.h"

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/datamessage.h>
#include <core/messaging/messages/replymessage.h>
#include <core/messaging/messages/learningmessage.h>
#include <core/messaging/messages/learningreplymessage.h>

namespace KyoukoMind
{

MessageTest::MessageTest()
{

}

void MessageTest::initTestCase()
{

}

void MessageTest::checkDataMessage()
{
    DataMessage message(1, 2, 3);

    CommonMessageData metaData = message.getMetaData();

    QCOMPARE(metaData.messageId, 8589934593);
    QCOMPARE(metaData.requiredReply, 1);
    QCOMPARE(metaData.site, 3);
    QCOMPARE(metaData.type, DATAMESSAGE);

    KyoChanEdge testEdge;
    testEdge.targetClusterId = 1;
    testEdge.targetNodeId = 2;
    testEdge.weight = 3;

    KyoChanEdge testEdge2;
    testEdge2.targetClusterId = 4;
    testEdge2.targetNodeId = 5;
    testEdge2.weight = 6;

    QCOMPARE(message.addEdge(testEdge), true);
    QCOMPARE(message.addEdge(testEdge2), true);

    QByteArray data = message.convertToByteArray();
    quint32 theoreticalSize = sizeof(CommonMessageData) + sizeof(KyoChanEdge) * 2 + 1;
    QCOMPARE(data.length(), theoreticalSize);

    DataMessage newMessage;
    newMessage.convertFromByteArray(data);

    CommonMessageData metaData2 = newMessage.getMetaData();

    QCOMPARE(metaData2.messageId, 8589934593);
    QCOMPARE(metaData2.requiredReply, 1);
    QCOMPARE(metaData2.site, 3);
    QCOMPARE(metaData2.type, DATAMESSAGE);

    QCOMPARE(newMessage.getNumberOfEdges(), 2);
    KyoChanEdge* edges = newMessage.getEdges();

    QCOMPARE(edges[0].targetClusterId, 1);
    QCOMPARE(edges[0].targetNodeId, 2);
    QCOMPARE(edges[0].weight, 3);
}

void MessageTest::checkReplyMessage()
{
    ReplyMessage message(1, 3);

    CommonMessageData metaData = message.getMetaData();

    QCOMPARE(metaData.messageId, 1);
    QCOMPARE(metaData.requiredReply, 0);
    QCOMPARE(metaData.site, 3);
    QCOMPARE(metaData.type, REPLYMESSAGE);
}

void MessageTest::checkLerningMessage()
{
    LearningMessage message(1, 2, 3);

    CommonMessageData metaData = message.getMetaData();

    QCOMPARE(metaData.messageId, 8589934593);
    QCOMPARE(metaData.requiredReply, 1);
    QCOMPARE(metaData.site, 3);
    QCOMPARE(metaData.type, LEARNINGMESSAGE);

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

    QCOMPARE(message.addNewEdge(testEdge), true);
    QCOMPARE(message.addNewEdge(testEdge2), true);

    QByteArray data = message.convertToByteArray();
    quint32 theoreticalSize = sizeof(CommonMessageData) + sizeof(KyoChanNewEdge) * 2 + 1;
    QCOMPARE(data.length(), theoreticalSize);

    LearningMessage newMessage;
    newMessage.convertFromByteArray(data);

    CommonMessageData metaData2 = newMessage.getMetaData();

    QCOMPARE(metaData2.messageId, 8589934593);
    QCOMPARE(metaData2.requiredReply, 1);
    QCOMPARE(metaData2.site, 3);
    QCOMPARE(metaData2.type, LEARNINGMESSAGE);

    QCOMPARE(newMessage.getNumberOfEdges(), 2);
    KyoChanNewEdge* edges = newMessage.getNewEdges();

    QCOMPARE(edges[0].weight, 1);
    QCOMPARE(edges[0].newEdgeId, 2);
    QCOMPARE(edges[0].sourceClusterId, 3);
    QCOMPARE(edges[0].sourceNodeId, 4);
}

void MessageTest::checkLearingReplyMessage()
{
    LearningReplyMessage message(1, 2, 3);

    CommonMessageData metaData = message.getMetaData();

    QCOMPARE(metaData.messageId, 8589934593);
    QCOMPARE(metaData.requiredReply, 1);
    QCOMPARE(metaData.site, 3);
    QCOMPARE(metaData.type, LEARNINGREPLYMESSAGE);

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

    QCOMPARE(message.addNewEdgeReply(testEdge), true);
    QCOMPARE(message.addNewEdgeReply(testEdge2), true);

    QByteArray data = message.convertToByteArray();
    quint32 theoreticalSize = sizeof(CommonMessageData) + sizeof(KyoChanNewEdgeReply) * 2 + 1;
    QCOMPARE(data.length(), theoreticalSize);

    LearningReplyMessage newMessage;
    newMessage.convertFromByteArray(data);

    CommonMessageData metaData2 = newMessage.getMetaData();

    QCOMPARE(metaData2.messageId, 8589934593);
    QCOMPARE(metaData2.requiredReply, 1);
    QCOMPARE(metaData2.site, 3);
    QCOMPARE(metaData2.type, LEARNINGREPLYMESSAGE);

    QCOMPARE(newMessage.getNumberOfEdgeReplys(), 2);
    KyoChanNewEdgeReply* edges = newMessage.getNewEdgeReplys();

    QCOMPARE(edges[0].newEdgeId, 1);
    QCOMPARE(edges[0].targetClusterId, 2);
    QCOMPARE(edges[0].targetNodeId, 3);
    QCOMPARE(edges[0].sourceClusterId, 4);
    QCOMPARE(edges[0].sourceNodeId, 5);
}

void MessageTest::cleanupTestCase()
{

}

}
