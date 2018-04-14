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

namespace KyoukoMind
{

/**
 * @brief DemoIO::DemoIO
 */
DemoIO::DemoIO(MessageController *messageController)
{
    m_messageController = messageController;
    ClusterID fakeClusterID = 1337;
    m_incomBuffer = new IncomingMessageBuffer(fakeClusterID, m_messageController);
    m_ougoingBuffer = new OutgoingMessageBuffer(fakeClusterID, m_messageController);
}

/**
 * @brief DemoIO::run
 */
void DemoIO::run()
{
    while(!m_abort)
    {
        usleep(PROCESS_INTERVAL);

        std::vector<Message*>* incomList = m_incomBuffer->getMessageQueue(15);
        for(uint32_t i = 0; i < incomList->size(); i++)
        {
            if((*incomList)[i]->getMetaData().type == DATA_MESSAGE) {
                DataMessage* dataM = static_cast<DataMessage*>((*incomList)[i]);

                KyoChanEdge* edges = dataM->getEdges();
                for(uint32_t j = 0; j < dataM->getNumberOfEdges(); j++)
                {
                    uint32_t out = (uint32_t)edges[j].weight;
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

    KyoChanEdge edge1;
    edge1.weight = (float)inputNumber;
    edge1.targetNodeId = 12;
    sendData(edge1);

    KyoChanEdge edge2;
    edge2.weight = (float)inputNumber;
    edge2.targetNodeId = 23;
    sendData(edge2);

    KyoChanEdge edge3;
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

    KyoChanEdge edge1;
    edge1.weight = (float)inputNumber;
    edge1.targetNodeId = 12;
    sendData(edge1);

    KyoChanEdge edge2;
    edge2.weight = (float)inputNumber;
    edge2.targetNodeId = 23;
    sendData(edge2);
}

/**
 * @brief DemoIO::sendData
 * @param input
 */
void DemoIO::sendData(const KyoChanEdge edge)
{
    mutexLock();
    m_ougoingBuffer->addEdge(11, 0, edge);
    mutexUnlock();
}

}
