/**
 *  @file    demoIO.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "demoIO.h"
#include <messages/dataMessage.h>

#include <messageQueues/incomingMessageBuffer.h>
#include <messageQueues/outgoingMessageBuffer.h>

#include <core/clustering/clusterHandler.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

namespace KyoukoMind
{

/**
 * @brief DemoIO::DemoIO
 */
DemoIO::DemoIO(ClusterHandler *clusterHandler)
{
    m_clusterHandler = clusterHandler;

    m_fakeCluster = new NodeCluster(1337, "/tmp/test", 2);
    m_clusterHandler->addCluster(1337, m_fakeCluster, false);
    m_fakeCluster->setNeighbor(16, 15);
    m_fakeCluster->setNeighbor(0, 17);


    Cluster* outgoingCluster = clusterHandler->getCluster(17);
    outgoingCluster->setNeighbor(16, 1337);
    outgoingCluster->setNewConnection(16, m_fakeCluster->getIncomingMessageBuffer(0));

    Cluster* ingoingCluster = clusterHandler->getCluster(15);
    ingoingCluster->setNeighbor(0, 1337);
    m_fakeCluster->setNewConnection(16, ingoingCluster->getIncomingMessageBuffer(0));

    m_incomBuffer = m_fakeCluster->getIncomingMessageBuffer(0);
    m_ougoingBuffer = m_fakeCluster->getOutgoingMessageBuffer(16);
}

/**
 * @brief DemoIO::run
 */
void DemoIO::run()
{
    while(!m_abort)
    {
        usleep(PROCESS_INTERVAL);

        uint8_t* data = (uint8_t*)m_incomBuffer->getMessage()->getPayload();
        uint8_t* end = data + m_incomBuffer->getMessage()->getPayloadSize();

        float out = 0;

        while(data < end)
        {
            if((int)(*data) == DIRECT_EDGE_CONTAINER) {
                KyoChanDirectEdgeContainer* edge = (KyoChanDirectEdgeContainer*)data;
                if(edge->targetNodeId == 0) {
                    out += (uint32_t)edge->weight;
                } else {
                    if(out - (uint32_t)edge->weight >= 0.0f) {
                        out -= (uint32_t)edge->weight;
                    } else {
                        out = 0.0f;
                    }
                }
                data += sizeof(KyoChanDirectEdgeContainer);
            }
        }

        if(out > 255.0) {
            out = 255.0;
        }
        if(out > 0.0) {
            std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! out number: "<<(int)out<<std::endl;
        }

        char newChar = (char)out;

        //sendInnerData(newChar);
    }
}

/**
 * @brief DemoIO::sendOutData
 * @param input
 */
void DemoIO::sendOutData(const char input)
{
    //std::cout<<"----------------------------------------- sendOutData"<<std::endl;
    uint8_t inputNumber = (uint8_t)input;

    mutexLock();
    KyoChanDirectEdgeContainer edge1;
    edge1.weight = (float)inputNumber;
    edge1.targetNodeId = 1;
    m_ougoingBuffer->addData(&edge1);

    KyoChanDirectEdgeContainer edge2;
    edge2.weight = (float)inputNumber;
    edge2.targetNodeId = 2;
    m_ougoingBuffer->addData(&edge2);

    KyoChanDirectEdgeContainer edge3;
    edge3.weight = (float)inputNumber;
    edge3.targetNodeId = 3;
    m_ougoingBuffer->addData(&edge3);

    m_fakeCluster->finishCycle(0);
    mutexUnlock();
}

/**
 * @brief DemoIO::sendInnerData
 * @param input
 */
void DemoIO::sendInnerData(const char input)
{
    return;
    std::cout<<"+++++++++++++++++++++++++++++++++++++ sendInnerData"<<std::endl;
    uint8_t inputNumber = (uint8_t)input;

    mutexLock();
    KyoChanDirectEdgeContainer edge1;
    edge1.weight = (float)inputNumber;
    edge1.targetNodeId = 1;
    m_ougoingBuffer->addData(&edge1);

    KyoChanDirectEdgeContainer edge2;
    edge2.weight = (float)inputNumber;
    edge2.targetNodeId = 2;
    m_ougoingBuffer->addData(&edge2);

    m_fakeCluster->finishCycle(0);
    mutexUnlock();
}

/**
 * @brief DemoIO::sendFinishCycle
 */
void DemoIO::sendFinishCycle()
{
    mutexLock();
    m_fakeCluster->finishCycle(0);
    mutexUnlock();
}

/**
 * @brief DemoIO::sendData
 * @param input
 */
void DemoIO::sendData(const KyoChanDirectEdgeContainer &edge)
{
    mutexLock();
    m_ougoingBuffer->addData(&edge);
    mutexUnlock();
}

}
