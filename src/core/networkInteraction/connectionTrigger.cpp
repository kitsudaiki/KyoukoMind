/**
 *  @file    connectionTrigger.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "connectionTrigger.h"

#include <kyoukoNetwork.h>

#include <core/bricks/brickHandler.h>
#include <core/bricks/brickObjects/brick.h>
#include <core/bricks/brickMethods/commonBrickMethods.h>
#include <core/bricks/globalValuesHandler.h>

#include <core/processing/processingMethods/messageProcessingMethods.h>
#include <core/processing/processingMethods/brickProcessingMethods.h>

#include <core/messaging/messageMarker/incomingBuffer.h>
#include <core/messaging/messageMarker/outgoingBuffer.h>
#include <core/messaging/messageObjects/messageBlock.h>
#include <core/messaging/messageObjects/messages.h>
#include <core/messaging/messageObjects/contentContainer.h>

#include <tcp/tcpClient.h>
#include <iostream>

using Kitsune::Chan::Communication::ClientRegisterInput;
using Kitsune::Chan::Communication::ClientRegisterOutput;
using Kitsune::Chan::Communication::ClientLearnInputData;
using Kitsune::Chan::Communication::ClientControlGlia;
using Kitsune::Chan::Communication::ClientControlLearning;
using Kitsune::Chan::Communication::ClientControlMemorizing;
using Kitsune::Chan::Communication::ClientControlOutputLearning;


namespace KyoukoMind
{

ConnectionTrigger::ConnectionTrigger(): Kitsune::Network::NetworkTrigger ()
{
    std::cout<<"init trigger"<<std::endl;
}

ConnectionTrigger::~ConnectionTrigger()
{

}

/**
 * process data coming from the tcp-socket
 *
 * @return number of processed bytes
 */
uint32_t
ConnectionTrigger::runTask(const MessageRingBuffer &recvBuffer,
                           Kitsune::Network::TcpClient *client)
{
    uint32_t processedBytes = 0;
    while(true)
    {
        // break-condition 1
        if(recvBuffer.readWriteDiff - processedBytes < 3) {
            break;
        }

        const uint32_t bufferPosition = (recvBuffer.readPosition + processedBytes)
                                        % recvBuffer.totalBufferSize;
        const uint8_t type = recvBuffer.data[bufferPosition];

        // get message-size
        const uint32_t messageSize = m_messageSize.sizes[type];
        if(type == UNDEFINED_TYPE)
        {
            std::cout<<"epic-fail"<<std::endl;
            break;
        }

        // break-condition 2
        if(recvBuffer.readWriteDiff - processedBytes < messageSize) {
            break;
        }

        // get data-block
        const uint8_t* dataPointer = getDataPointer(recvBuffer,
                                                    m_tempBuffer,
                                                    messageSize,
                                                    processedBytes);
        processedBytes += messageSize;

        if(type == CLIENT_REGISTER_INPUT)
        {
            std::cout<<"CLIENT_REGISTER_INPUT"<<std::endl;
            const ClientRegisterInput content = *((ClientRegisterInput*)dataPointer);

            Brick* incomingBrick = KyoukoNetwork::m_brickHandler->getBrick(content.brickId);
            addClientConnection(incomingBrick, true, false);
        }

        if(type == CLIENT_REGISTER_OUTPUT)
        {
            std::cout<<"CLIENT_REGISTER_OUTPUT"<<std::endl;
            const ClientRegisterOutput content = *((ClientRegisterOutput*)dataPointer);

            Brick* outgoingBrick = KyoukoNetwork::m_brickHandler->getBrick(content.brickId);
            addClientConnection(outgoingBrick, false, true);
        }

        if(type == CLIENT_CONTROL_LEARNING)
        {
            std::cout<<"CLIENT_CONTROL_LEARNING"<<std::endl;
            const ClientControlLearning content = *((ClientControlLearning*)dataPointer);

            GlobalValuesHandler* handler = KyoukoNetwork::m_globalValuesHandler;
            GlobalValues gValues = handler->getGlobalValues();
            gValues.globalLearningTemp = content.learnTemp;
            gValues.globalLearningOffset = content.learnOffset;
            handler->setGlobalValues(gValues);
        }

        if(type == CLIENT_CONTROL_MEMORIZING)
        {
            std::cout<<"CLIENT_CONTROL_MEMORIZING"<<std::endl;
            ClientControlMemorizing content = *((ClientControlMemorizing*)dataPointer);

            GlobalValuesHandler* handler = KyoukoNetwork::m_globalValuesHandler;
            GlobalValues gValues = handler->getGlobalValues();
            gValues.globalMemorizingTemp = content.memTemp;
            gValues.globalMemorizingOffset = content.memOffset;
            handler->setGlobalValues(gValues);
        }

        if(type == CLIENT_CONTROL_GLIA)
        {
            std::cout<<"CLIENT_CONTROL_GLIA"<<std::endl;
            const ClientControlGlia content = *((ClientControlGlia*)dataPointer);

            GlobalValuesHandler* handler = KyoukoNetwork::m_globalValuesHandler;
            GlobalValues gValues = handler->getGlobalValues();
            gValues.globalGlia = content.glia;
            handler->setGlobalValues(gValues);
        }

        if(type == CLIENT_CONTROL_OUTPUT_LEARNING)
        {
            std::cout<<"CLIENT_CONTROL_OUTPUT_LEARNING"<<std::endl;
            const ClientControlOutputLearning content = *((ClientControlOutputLearning*)dataPointer);
            Brick* brick = KyoukoNetwork::m_brickHandler->getBrick(content.brickId);

            if(brick != nullptr) {
                brick->learningOverride = content.outputLearning;
            }
        }

        if(type == CLIENT_LEARN_INPUT)
        {
            //std::cout<<"CLIENT_LEARN_INPUT"<<std::endl;
            const ClientLearnInputData content = *((ClientLearnInputData*)dataPointer);
            Brick* brick = KyoukoNetwork::m_brickHandler->getBrick(content.brickId);
            if(brick->dataConnections[NODE_DATA].inUse > 0)
            {
                for(uint16_t i = 0; i < NUMBER_OF_NODES_PER_BRICK; i++)
                {
                    DirectEdgeContainer edge;
                    edge.weight = content.value;
                    edge.targetNodeId = i;
                    sendData(brick, 24, edge);
                }
            }
        }
    }

    return processedBytes;
}

}
