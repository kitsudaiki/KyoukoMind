/**
 *  @file    kyouko_network.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <root_object.h>
#include <core/network_manager.h>
#include <core/global_values_handler.h>
#include <core/objects/brick.h>
#include <core/objects/container_definitions.h>
#include <core/processing/processing_methods/brick_initializing_methods.h>

#include <libKitsunemimiPersistence/logger/logger.h>

#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>

#include <libKitsunemimiKyoukoCommon/communication_structs/mind_container.h>
#include <libKitsunemimiKyoukoCommon/communication_structs/monitoring_contianer.h>
#include <libKitsunemimiKyoukoCommon/communication_structs/client_contianer.h>

using Kitsunemimi::Kyouko::ClientRegisterInput;
using Kitsunemimi::Kyouko::ClientRegisterOutput;
using Kitsunemimi::Kyouko::ClientControlLearning;
using Kitsunemimi::Kyouko::ClientControlMemorizing;
using Kitsunemimi::Kyouko::ClientControlGlia;
using Kitsunemimi::Kyouko::ClientControlOutputLearning;
using Kitsunemimi::Kyouko::ClientLearnInputData;

namespace KyoukoMind
{

// init static variables
KyoukoMind::Config* RootObject::m_config = nullptr;
KyoukoMind::BrickHandler* RootObject::m_brickHandler = nullptr;
KyoukoMind::GlobalValuesHandler* RootObject::m_globalValuesHandler = nullptr;
Kitsunemimi::Project::Session* RootObject::m_clientSession = nullptr;
Kitsunemimi::Project::Session* RootObject::m_monitoringSession = nullptr;

/**
 * @brief streamDataCallback
 * @param target
 * @param data
 * @param dataSize
 */
void streamDataCallback(void* target,
                        Kitsunemimi::Project::Session*,
                        const void* data,
                        const uint64_t dataSize)
{
    RootObject* rootObject = static_cast<RootObject*>(target);
    const uint8_t* dataObj = static_cast<const uint8_t*>(data);

    uint64_t dataPos = 0;

    while(dataPos < dataSize)
    {
        const uint8_t type = dataObj[dataPos];

        if(type == CLIENT_REGISTER_INPUT)
        {
            std::cout<<"CLIENT_REGISTER_INPUT"<<std::endl;
            const ClientRegisterInput content = *((ClientRegisterInput*)&dataObj[dataPos]);

            Brick* incomingBrick = rootObject->m_brickHandler->getBrick(content.brickId);
            addClientConnection(*incomingBrick, true, false);
            dataPos += sizeof(ClientRegisterInput);
        }

        if(type == CLIENT_REGISTER_OUTPUT)
        {
            std::cout<<"CLIENT_REGISTER_OUTPUT"<<std::endl;
            const ClientRegisterOutput content = *((ClientRegisterOutput*)&dataObj[dataPos]);

            Brick* outgoingBrick = rootObject->m_brickHandler->getBrick(content.brickId);
            addClientConnection(*outgoingBrick, false, true);
            dataPos += sizeof(ClientRegisterOutput);
        }

        if(type == CLIENT_CONTROL_LEARNING)
        {
            std::cout<<"CLIENT_CONTROL_LEARNING"<<std::endl;
            const ClientControlLearning content = *((ClientControlLearning*)&dataObj[dataPos]);

            GlobalValuesHandler* handler = rootObject->m_globalValuesHandler;
            GlobalValues gValues = handler->getGlobalValues();
            gValues.globalLearningTemp = content.learnTemp;
            gValues.globalLearningOffset = content.learnOffset;
            handler->setGlobalValues(gValues);
            dataPos += sizeof(ClientControlLearning);
        }

        if(type == CLIENT_CONTROL_MEMORIZING)
        {
            std::cout<<"CLIENT_CONTROL_MEMORIZING"<<std::endl;
            ClientControlMemorizing content = *((ClientControlMemorizing*)&dataObj[dataPos]);

            GlobalValuesHandler* handler = rootObject->m_globalValuesHandler;
            GlobalValues gValues = handler->getGlobalValues();
            gValues.globalMemorizingTemp = content.memTemp;
            gValues.globalMemorizingOffset = content.memOffset;
            handler->setGlobalValues(gValues);
            dataPos += sizeof(ClientControlMemorizing);
        }

        if(type == CLIENT_CONTROL_GLIA)
        {
            std::cout<<"CLIENT_CONTROL_GLIA"<<std::endl;
            const ClientControlGlia content = *((ClientControlGlia*)&dataObj[dataPos]);

            GlobalValuesHandler* handler = rootObject->m_globalValuesHandler;
            GlobalValues gValues = handler->getGlobalValues();
            gValues.globalGlia = content.glia;
            handler->setGlobalValues(gValues);
            dataPos += sizeof(ClientControlGlia);
        }

        if(type == CLIENT_CONTROL_OUTPUT_LEARNING)
        {
            std::cout<<"CLIENT_CONTROL_OUTPUT_LEARNING"<<std::endl;
            const ClientControlOutputLearning content
                    = *((ClientControlOutputLearning*)&dataObj[dataPos]);
            Brick* brick = rootObject->m_brickHandler->getBrick(content.brickId);

            if(brick != nullptr) {
                brick->learningOverride = content.outputLearning;
            }
            dataPos += sizeof(ClientControlOutputLearning);
        }

        if(type == CLIENT_LEARN_INPUT)
        {
            //std::cout<<"CLIENT_LEARN_INPUT"<<std::endl;
            const ClientLearnInputData content = *((ClientLearnInputData*)&dataObj[dataPos]);
            Brick* brick = rootObject->m_brickHandler->getBrick(content.brickId);
            if(brick->dataConnections[NODE_DATA].inUse > 0)
            {
                for(uint16_t i = 0; i < NUMBER_OF_NODES_PER_BRICK; i++)
                {
                    DirectEdgeContainer edge;
                    edge.weight = content.value;
                    edge.targetNodeId = i;
                    //sendData(brick, 24, edge);
                }
            }
            dataPos += sizeof(ClientLearnInputData);
        }
    }
}

/**
 * @brief standaloneDataCallback
 * @param target
 * @param data
 * @param dataSize
 */
void standaloneDataCallback(void* target,
                            Kitsunemimi::Project::Session*,
                            const uint64_t,
                            DataBuffer* data)
{

    RootObject* rootObject = static_cast<RootObject*>(target);

    delete data;
}

/**
 * @brief errorCallback
 */
void errorCallback(void*,
                   Kitsunemimi::Project::Session*,
                   const uint8_t,
                   const std::string message)
{
    LOG_ERROR("error-callback: " + message);
}

/**
 * @brief sessionCallback
 * @param target
 * @param isInit
 * @param session
 * @param sessionIdentifier
 */
void sessionCallback(void* target,
                     bool isInit,
                     Kitsunemimi::Project::Session* session,
                     const std::string sessionIdentifier)
{
    RootObject* rootObject = static_cast<RootObject*>(target);
    if(isInit)
    {
        if(sessionIdentifier == "client") {
            rootObject->m_clientSession = session;
        }
        if(sessionIdentifier == "monitoring") {
            rootObject->m_monitoringSession = session;
        }
    }
    else
    {
        if(session->m_sessionIdentifier == "client")
        {
            delete rootObject->m_clientSession;
            rootObject->m_clientSession = nullptr;
        }
        if(session->m_sessionIdentifier == "monitoring")
        {
            delete rootObject->m_monitoringSession;
            rootObject->m_clientSession = nullptr;
        }
    }
}

/**
 * main-class
 */
RootObject::RootObject()
{
    m_config = new Config();
    m_brickHandler = new BrickHandler();
    m_globalValuesHandler = new GlobalValuesHandler();
}

/**
 * init all components
 */
void
RootObject::start()
{
    // network-manager
    m_networkManager = new NetworkManager();
}

} // namespace KyoukoMind
