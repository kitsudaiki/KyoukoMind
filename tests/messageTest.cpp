/**
 *  @file    messageTest.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "messageTest.h"

#include <messages/message.h>
#include <messages/dataMessage.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{

MessageTest::MessageTest() : Kitsune::CommonTest("MessageTest")
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
    Networking::DataMessage message(1, 3, 4, 1);

    DataMessageHeader metaData = message.getMetaData();

    UNITTEST(metaData.requiredReply, 1);
    UNITTEST((uint32_t)metaData.commonInfo.targetSide, 3);
    UNITTEST(metaData.commonInfo.type, DATA_MESSAGE);

    KyoChanForwardEdgeContainer testEdge;
    testEdge.targetEdgeSectionId = 1;
    testEdge.weight = 3;
    message.addData((void*)(&testEdge), sizeof(KyoChanForwardEdgeContainer));

    KyoChanForwardEdgeContainer testEdge2;
    testEdge2.targetEdgeSectionId = 4;
    testEdge2.weight = 6;
    message.addData((void*)(&testEdge2), sizeof(KyoChanForwardEdgeContainer));

    /*void* data = message.getData();
    uint32_t size = message.getDataSize();

    Networking::DataMessage newMessage(data, size);
    newMessage.getMetaDataFromBuffer();
    DataMessageHeader metaData2 = newMessage.getMetaData();

    UNITTEST((uint8_t)metaData2.requiredReply, 1);
    UNITTEST((uint32_t)metaData2.commonInfo.targetSide, 3);
    UNITTEST(metaData2.commonInfo.type, DATA_MESSAGE);

    uint8_t* newData = (uint8_t*)newMessage.getPayload();

    UNITTEST(newData[0], FOREWARD_EDGE_CONTAINER);

    KyoChanForwardEdgeContainer* edges = (KyoChanForwardEdgeContainer*)newData;

    UNITTEST(edges[0].targetEdgeSectionId, 1);
    UNITTEST(edges[0].weight, 3);*/
}

void MessageTest::checkReplyMessage()
{
}

void MessageTest::cleanupTestCase()
{

}

}
