/**
 *  @file    connectionTrigger.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "connectionTrigger.h"

#include <core/clustering/clusterHandler.h>
#include <core/clustering/cluster/nodeCluster.h>
#include <core/clustering/globalValuesHandler.h>

#include <communicationEnums.h>
#include <communicationStructs/mindContainer.h>
#include <communicationStructs/clientContianer.h>

#include <core/messaging/incomingMessageBuffer.h>
#include <core/messaging/outgoingMessageBuffer.h>
#include <core/messaging/messages/dataMessage.h>

#include <tcp/tcpClient.h>
#include <iostream>

using Kitsune::Chan::Communication::ClientRegisterOutput;
using Kitsune::Chan::Communication::ClientLearnInputData;
using Kitsune::Chan::Communication::ClientControlGlia;
using Kitsune::Chan::Communication::ClientControlLearning;
using Kitsune::Chan::Communication::ClientControlMemorizing;
using Kitsune::Chan::Communication::ClientControlOutputLearning;


namespace KyoukoMind
{

ConnectionTrigger::ConnectionTrigger(ClusterHandler* model): Kitsune::Network::NetworkTrigger ()
{
    m_model = model;
    std::cout<<"init trigger"<<std::endl;
}

ConnectionTrigger::~ConnectionTrigger()
{

}

/**
 * @brief ConnectionTrigger::runTask
 * @param buffer
 * @param bufferSize
 * @param client
 */
void
ConnectionTrigger::runTask(uint8_t *buffer,
                           const long bufferSize,
                           Kitsune::Network::TcpClient *client)
{
    uint32_t pos = 0;
    while(pos < bufferSize)
    {
        if(buffer[pos] == CLIENT_REGISTER_OUTPUT)
        {
            if(pos + sizeof(ClientRegisterOutput) > bufferSize) {
                return;
            }
            std::cout<<"CLIENT_REGISTER_OUTPUT"<<std::endl;
            ClientRegisterOutput content = *((ClientRegisterOutput*)(buffer + pos));

            NodeCluster* outgoingCluster = static_cast<NodeCluster*>(m_model->getCluster(content.clusterId));
            outgoingCluster->setAsOutput();

            pos += sizeof(ClientRegisterOutput);
        }

        if(buffer[pos] == CLIENT_CONTROL_LEARNING)
        {
            if(pos + sizeof(ClientControlLearning) > bufferSize) {
                return;
            }
            std::cout<<"CLIENT_CONTROL_LEARNING"<<std::endl;
            ClientControlLearning content = *((ClientControlLearning*)(buffer + pos));

            GlobalValuesHandler* handler = m_model->getGlobalValuesHandler();
            GlobalValues gValues = handler->getGlobalValues();
            gValues.globalLearningTemp = content.learnTemp;
            gValues.globalLearningOffset = content.learnOffset;
            handler->setGlobalValues(gValues);

            pos += sizeof(ClientControlLearning);
        }


        if(buffer[pos] == CLIENT_CONTROL_MEMORIZING)
        {
            if(pos + sizeof(ClientControlMemorizing) > bufferSize) {
                return;
            }
            std::cout<<"CLIENT_CONTROL_MEMORIZING"<<std::endl;
            ClientControlMemorizing content = *((ClientControlMemorizing*)(buffer + pos));

            GlobalValuesHandler* handler = m_model->getGlobalValuesHandler();
            GlobalValues gValues = handler->getGlobalValues();
            gValues.globalMemorizingTemp = content.memTemp;
            gValues.globalMemorizingOffset = content.memOffset;
            handler->setGlobalValues(gValues);

            pos += sizeof(ClientControlMemorizing);
        }

        if(buffer[pos] == CLIENT_CONTROL_GLIA)
        {
            if(pos + sizeof(ClientControlGlia) > bufferSize) {
                return;
            }
            std::cout<<"CLIENT_CONTROL_GLIA"<<std::endl;
            ClientControlGlia content = *((ClientControlGlia*)(buffer + pos));

            GlobalValuesHandler* handler = m_model->getGlobalValuesHandler();
            GlobalValues gValues = handler->getGlobalValues();
            gValues.globalGlia = content.glia;
            handler->setGlobalValues(gValues);

            pos += sizeof(ClientControlGlia);
        }

        if(buffer[pos] == CLIENT_CONTROL_OUTPUT_LEARNING)
        {
            if(pos + sizeof(ClientControlOutputLearning) > bufferSize) {
                return;
            }
            std::cout<<"CLIENT_CONTROL_OUTPUT_LEARNING"<<std::endl;
            ClientControlOutputLearning content = *((ClientControlOutputLearning*)(buffer + pos));
            Cluster* cluster = m_model->getCluster(content.clusterId);
            if(cluster != nullptr) {
                cluster->setLearningOverride(content.outputLearning);
            }
        }

        if(buffer[pos] == CLIENT_LEARN_INPUT)
        {
            if(pos + sizeof(ClientLearnInputData) > bufferSize) {
                return;
            }
            ClientLearnInputData content = *((ClientLearnInputData*)(buffer + pos));

            IncomingMessageBuffer* buffer = m_model->getCluster(content.clusterId)->getIncomingMessageBuffer(0);
            if(buffer == nullptr) {
                buffer = new IncomingMessageBuffer();
                m_model->getCluster(content.clusterId)->setIncomBuffer(0, buffer);
            }

            DataMessage* message = new DataMessage(content.clusterId,0,0,0);
            for(uint16_t i = 0; i < NUMBER_OF_NODES_PER_CLUSTER; i++)
            {
                KyoChanDirectEdgeContainer edge;
                edge.weight = content.value;
                edge.targetNodeId = i;
                message->addData((void*)(&edge), sizeof(KyoChanDirectEdgeContainer));
            }
            buffer->addMessage(message);

            pos += sizeof(ClientLearnInputData);
        }
    }
}

}
