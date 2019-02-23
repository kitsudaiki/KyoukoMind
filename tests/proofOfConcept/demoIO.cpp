/**
 *  @file    demoIO.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "demoIO.h"
#include <core/messaging/messages/dataMessage.h>

#include <core/messaging/incomingMessageBuffer.h>
#include <core/messaging/outgoingMessageBuffer.h>

#include <core/clustering/clusterHandler.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/clustering/globalValuesHandler.h>

namespace KyoukoMind
{

/**
 * @brief DemoIO::DemoIO
 */
DemoIO::DemoIO(ClusterHandler *clusterHandler)
{
    m_clusterHandler = clusterHandler;

    m_globalValuesHandler = m_clusterHandler->getGlobalValuesHandler();

    uint32_t incomClusterId = 14;
    uint32_t outcomClusterId = 125;

    m_fakeCluster = new NodeCluster(1337, 0,0,"/tmp/test", 2);
    m_clusterHandler->addCluster(1337, m_fakeCluster, false);
    m_fakeCluster->setNeighbor(16, incomClusterId);
    m_fakeCluster->setNeighbor(0, outcomClusterId);


    Cluster* outgoingCluster = clusterHandler->getCluster(outcomClusterId);
    outgoingCluster->setNeighbor(16, 1337);
    outgoingCluster->setNewConnection(16, m_fakeCluster->getIncomingMessageBuffer(0));

    Cluster* ingoingCluster = clusterHandler->getCluster(incomClusterId);
    ingoingCluster->setNeighbor(0, 1337);
    m_fakeCluster->setNewConnection(16, ingoingCluster->getIncomingMessageBuffer(0));

    m_incomBuffer = m_fakeCluster->getIncomingMessageBuffer(0);
    m_ougoingBuffer = m_fakeCluster->getOutgoingMessageBuffer(16);
}

/**
 * @brief DemoIO::run
 */
void
DemoIO::run()
{
    while(!m_abort)
    {
        usleep(PROCESS_INTERVAL);

        DataMessage* message = m_incomBuffer->getMessage();
        if(message == nullptr) {
            continue;
        }

        uint8_t* data = (uint8_t*)message->getPayload();
        uint8_t* end = data + message->getPayloadSize();

        float outputValues[3];
        outputValues[0] = 0.0f;
        outputValues[1] = 0.0f;
        outputValues[2] = 0.0f;

        while(data < end)
        {
            if((int)(*data) == DIRECT_EDGE_CONTAINER)
            {
                KyoChanDirectEdgeContainer* edge = (KyoChanDirectEdgeContainer*)data;

                outputValues[edge->targetNodeId % 3] += edge->weight;

                data += sizeof(KyoChanDirectEdgeContainer);
            }
        }
        m_incomBuffer->finish();

        char outputText = convertOutput(outputValues[0], outputValues[1], outputValues[2]);
        if(outputText != ' ')
        {
            std::cout<<"        output: "<<outputText<<std::endl;
            //sendInnerData(outputText);
        }
    }
}

/**
 * @brief DemoIO::sendOutData
 * @param input
 */
void
DemoIO::sendOutData(const char input)
{
    //std::cout<<"----------------------------------------- sendOutData"<<std::endl;
    uint8_t inputNumber = (uint8_t)input;
    NeighborInformation neighborInfo;

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

    m_fakeCluster->finishCycle();
    mutexUnlock();
}

/**
 * @brief DemoIO::sendPositive
 */
void DemoIO::sendPositive()
{
    GlobalValues globalValues = m_globalValuesHandler->getGlobalValues();
    globalValues.globalMemorizingOffset += 1.0f * MEMORIZING_COOLDOWN;
    m_globalValuesHandler->setGlobalValues(globalValues);
}

/**
 * @brief DemoIO::sendNegative
 */
void DemoIO::sendNegative()
{
    GlobalValues globalValues = m_globalValuesHandler->getGlobalValues();
    globalValues.globalLearningOffset -= 1.0f * LEARNING_COOLDOWN;
    m_globalValuesHandler->setGlobalValues(globalValues);
}

/**
 * @brief DemoIO::resetLearning
 */
void DemoIO::resetLearning()
{
    GlobalValues globalValues;
    m_globalValuesHandler->setGlobalValues(globalValues);
}

/**
 * @brief DemoIO::convertOutput
 * @param one
 * @param two
 * @param three
 * @return
 */
char DemoIO::convertOutput(float one, float two, float three)
{
    if(two == 0.0f || three == 0.0f) {
        return ' ';
    }

    if(two >= one && three >= two) {
        return 'a';
    }

    if(two >= one && three < two) {
        return 's';
    }

    if(two < one && three >= two) {
        return 'd';
    }

    if(two < one && three < two) {
        return 'f';
    }

    return ' ';
}

/**
 * @brief DemoIO::sendInnerData
 * @param input
 */
void
DemoIO::sendInnerData(const char input)
{
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

    m_fakeCluster->finishCycle();
    mutexUnlock();
}

/**
 * @brief DemoIO::sendData
 * @param input
 */
void
DemoIO::sendData(const KyoChanDirectEdgeContainer &edge)
{
    mutexLock();
    m_ougoingBuffer->addData(&edge);
    mutexUnlock();
}

}
