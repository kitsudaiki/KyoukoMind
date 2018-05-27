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

#include <core/clustering/clusterHandler.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

namespace KyoukoMind
{

/**
 * @brief DemoIO::DemoIO
 */
DemoIO::DemoIO(MessageController *messageController,
               ClusterHandler *clusterHandler)
{
    m_clusterHandler = clusterHandler;
    m_messageController = messageController;
    NodeCluster* fakeCluster = new NodeCluster(1337, "/tmp/test");
    fakeCluster->initMessageBuffer(m_messageController);

    Neighbor neighborIn;
    neighborIn.targetClusterId = 12;
    neighborIn.neighborType = NODE_CLUSTER;
    fakeCluster->addNeighbor(16, neighborIn);

    EdgeCluster* outgoingCluster = clusterHandler->getCluster(14);

    Neighbor neighborOut;
    neighborOut.targetClusterId = 1337;
    neighborOut.neighborType = NODE_CLUSTER;
    outgoingCluster->addNeighbor(16, neighborOut);

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

        uint8_t* data = (uint8_t*)m_incomBuffer->getMessage(0)->getPayload();
        uint8_t* end = data + m_incomBuffer->getMessage(0)->getPayloadSize();

        while(data < end)
        {
            if((int)(*data) == DIRECT_EDGE_CONTAINER) {
                KyoChanForwardEdgeContainer* edge = (KyoChanForwardEdgeContainer*)data;

                uint32_t out = (uint32_t)edge->weight;
                if(out > 255) {
                    out = 255;
                }
                char newChar = (char)out;
                sendInnerData(newChar);
                data += sizeof(KyoChanStatusEdgeContainer);
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

    KyoChanDirectEdgeContainer edge1;
    edge1.weight = (float)inputNumber;
    edge1.targetNodeId = 1;
    sendData(edge1);

    KyoChanDirectEdgeContainer edge2;
    edge2.weight = (float)inputNumber;
    edge2.targetNodeId = 2;
    sendData(edge2);

    KyoChanDirectEdgeContainer edge3;
    edge3.weight = (float)inputNumber;
    edge3.targetNodeId = 3;
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

    KyoChanDirectEdgeContainer edge1;
    edge1.weight = (float)inputNumber;
    edge1.targetNodeId = 1;
    sendData(edge1);

    KyoChanDirectEdgeContainer edge2;
    edge2.weight = (float)inputNumber;
    edge2.targetNodeId = 2;
    sendData(edge2);

    sendFinishCycle();
}

/**
 * @brief DemoIO::sendFinishCycle
 */
void DemoIO::sendFinishCycle()
{
    mutexLock();
    m_ougoingBuffer->finishCycle(16, 0);
    mutexUnlock();
}

/**
 * @brief DemoIO::sendData
 * @param input
 */
void DemoIO::sendData(const KyoChanDirectEdgeContainer &edge)
{
    mutexLock();
    m_ougoingBuffer->addDirectEdge(16, &edge);
    mutexUnlock();
}

}
