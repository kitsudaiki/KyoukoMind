/**
 *  @file    demoIO.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "demoIO.h"
#include <core/messaging/messageController.h>
#include <core/messaging/messages/dataMessage.h>

#include <core/messaging/messageQueues/messageBuffer.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

namespace KyoukoMind
{

/**
 * @brief DemoIO::DemoIO
 */
DemoIO::DemoIO(MessageController *messageController)
{
    m_messageController = messageController;
    EdgeCluster* fakeCluster = new NodeCluster(1337, "/tmp/test", 42);
    fakeCluster->initMessageBuffer(m_messageController);

    Neighbor neighbor;
    neighbor.targetClusterId = 12;
    neighbor.neighborType = NODE_CLUSTER;
    fakeCluster->addNeighbor(15, neighbor);

    m_incomBuffer = fakeCluster->getIncomingMessageBuffer();
    m_ougoingBuffer = fakeCluster->getOutgoingMessageBuffer();
}

/**
 * @brief DemoIO::run
 */
void DemoIO::run()
{
    while(!m_abort)
    {
        usleep(PROCESS_INTERVAL);

        Message* message = m_incomBuffer->getMessage(15);
        if(message != nullptr)
        {
            uint8_t* data = (uint8_t*)message->getData();
            for(uint32_t i = 0; i < message->getPayloadSize(); i = i + 20)
            {
                if(data[i] == FOREWARD_EDGE_CONTAINER) {
                    KyoChanForwardEdgeContainer* edge = (KyoChanForwardEdgeContainer*)data[i];

                    uint32_t out = (uint32_t)edge->weight;
                    if(out > 255) {
                        out = 255;
                    }
                    char newChar = (char)out;
                    sendInnerData(newChar);
                }
            }
        }
    }
}

/**
 * @brief DemoIO::sendOutData
 * @param input
 */
void DemoIO::sendOutData(const char input)
{
    uint8_t inputNumber = (uint8_t)input;
    OUTPUT("inputNumber:")
    OUTPUT((int)inputNumber)
    KyoChanForwardEdgeContainer edge1;
    edge1.weight = (float)inputNumber;
    edge1.targetEdgeSectionId = 1;
    sendData(edge1);

    KyoChanForwardEdgeContainer edge2;
    edge2.weight = (float)inputNumber;
    edge2.targetEdgeSectionId = 1;
    sendData(edge2);

    KyoChanForwardEdgeContainer edge3;
    edge3.weight = (float)inputNumber;
    edge3.targetEdgeSectionId = 1;
    sendData(edge3);
    sendFinishCycle();
}

/**
 * @brief DemoIO::sendInnerData
 * @param input
 */
void DemoIO::sendInnerData(const char input)
{
    uint8_t inputNumber = (uint8_t)input;

    KyoChanForwardEdgeContainer edge1;
    edge1.weight = (float)inputNumber;
    edge1.targetEdgeSectionId = 1;
    sendData(edge1);

    KyoChanForwardEdgeContainer edge2;
    edge2.weight = (float)inputNumber;
    edge2.targetEdgeSectionId = 1;
    sendData(edge2);
    sendFinishCycle();
}

/**
 * @brief DemoIO::sendFinishCycle
 */
void DemoIO::sendFinishCycle()
{
    mutexLock();
    m_ougoingBuffer->finishCycle(15, 0);
    mutexUnlock();
}

/**
 * @brief DemoIO::sendData
 * @param input
 */
void DemoIO::sendData(const KyoChanForwardEdgeContainer &edge)
{
    mutexLock();
    m_ougoingBuffer->addForwardEdge(15, &edge);
    mutexUnlock();
}

}