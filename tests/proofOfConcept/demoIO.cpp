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

#include <core/clustering/cluster/cluster.h>
#include <core/clustering/cluster/nodeCluster.h>

namespace KyoukoMind
{

/**
 * @brief DemoIO::DemoIO
 */
DemoIO::DemoIO(MessageController *messageController)
{
    m_messageController = messageController;
    Cluster* fakeCluster = new Cluster(1337, NODE_CLUSTER, "", m_messageController);

    Neighbor neighbor;
    neighbor.targetClusterId = 11;
    neighbor.side = 0;
    neighbor.neighborType = NODE_CLUSTER;
    fakeCluster->addNeighbor(0, neighbor);

    m_incomBuffer = new IncomingMessageBuffer(fakeCluster, m_messageController);
    m_ougoingBuffer = new OutgoingMessageBuffer(fakeCluster, m_messageController);
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
        uint8_t* data = (uint8_t*)message->getData();
        for(uint32_t i = 0; i < message->getPayloadSize(); i = i + 20)
        {
            if(data[i] == EDGE_CONTAINER) {
                KyoChanEdgeContainer* edge = (KyoChanEdgeContainer*)data[i];

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

/**
 * @brief DemoIO::sendOutData
 * @param input
 */
void DemoIO::sendOutData(const char input)
{
    uint8_t inputNumber = (uint8_t)input;

    KyoChanEdgeContainer edge1;
    edge1.weight = (float)inputNumber;
    edge1.targetNodeId = 12;
    sendData(edge1);

    KyoChanEdgeContainer edge2;
    edge2.weight = (float)inputNumber;
    edge2.targetNodeId = 23;
    sendData(edge2);

    KyoChanEdgeContainer edge3;
    edge3.weight = (float)inputNumber;
    edge3.targetNodeId = 42;
    sendData(edge3);
}

/**
 * @brief DemoIO::sendInnerData
 * @param input
 */
void DemoIO::sendInnerData(const char input)
{
    uint8_t inputNumber = (uint8_t)input;

    KyoChanEdgeContainer edge1;
    edge1.weight = (float)inputNumber;
    edge1.targetNodeId = 12;
    sendData(edge1);

    KyoChanEdgeContainer edge2;
    edge2.weight = (float)inputNumber;
    edge2.targetNodeId = 23;
    sendData(edge2);
}

/**
 * @brief DemoIO::sendData
 * @param input
 */
void DemoIO::sendData(const KyoChanEdgeContainer &edge)
{
    mutexLock();
    m_ougoingBuffer->addEdge(0, &edge);
    mutexUnlock();
}

}
