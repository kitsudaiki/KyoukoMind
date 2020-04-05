#ifndef NETWORK_CALLBACKS_H
#define NETWORK_CALLBACKS_H

#include <root_object.h>
#include <core/objects/container_definitions.h>
#include <core/processing/processing_methods/brick_initializing_methods.h>
#include <core/processing/processing_methods/brick_processing_methods.h>

#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>
#include <libKitsunemimiPersistence/logger/logger.h>

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

/**
 * @brief streamDataCallback
 * @param target
 * @param data
 * @param dataSize
 */
void
streamDataCallback(void* target,
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

        switch(type)
        {
            case CLIENT_REGISTER_INPUT:
            {
                LOG_DEBUG("CLIENT_REGISTER_INPUT");
                const ClientRegisterInput content = *((ClientRegisterInput*)&dataObj[dataPos]);

                Brick* incomingBrick = rootObject->m_brickHandler->getBrick(content.brickId);
                addClientConnection(*incomingBrick, true, false);
                dataPos += sizeof(ClientRegisterInput);
                break;
            }

            case CLIENT_REGISTER_OUTPUT:
            {
                LOG_DEBUG("CLIENT_REGISTER_OUTPUT");
                const ClientRegisterOutput content = *((ClientRegisterOutput*)&dataObj[dataPos]);

                Brick* outgoingBrick = rootObject->m_brickHandler->getBrick(content.brickId);
                addClientConnection(*outgoingBrick, false, true);
                dataPos += sizeof(ClientRegisterOutput);
                break;
            }

            case CLIENT_CONTROL_LEARNING:
            {
                LOG_DEBUG("CLIENT_CONTROL_LEARNING");
                const ClientControlLearning content = *((ClientControlLearning*)&dataObj[dataPos]);

                GlobalValuesHandler* handler = rootObject->m_globalValuesHandler;
                GlobalValues gValues = handler->getGlobalValues();
                gValues.globalLearningTemp = content.learnTemp;
                gValues.globalLearningOffset = content.learnOffset;
                handler->setGlobalValues(gValues);
                dataPos += sizeof(ClientControlLearning);
                break;
            }

            case CLIENT_CONTROL_MEMORIZING:
            {
                LOG_DEBUG("CLIENT_CONTROL_MEMORIZING");
                ClientControlMemorizing content = *((ClientControlMemorizing*)&dataObj[dataPos]);

                GlobalValuesHandler* handler = rootObject->m_globalValuesHandler;
                GlobalValues gValues = handler->getGlobalValues();
                gValues.globalMemorizingTemp = content.memTemp;
                gValues.globalMemorizingOffset = content.memOffset;
                handler->setGlobalValues(gValues);
                dataPos += sizeof(ClientControlMemorizing);
                break;
            }

            case CLIENT_CONTROL_GLIA:
            {
                LOG_DEBUG("CLIENT_CONTROL_GLIA");
                const ClientControlGlia content = *((ClientControlGlia*)&dataObj[dataPos]);

                GlobalValuesHandler* handler = rootObject->m_globalValuesHandler;
                GlobalValues gValues = handler->getGlobalValues();
                gValues.globalGlia = content.glia;
                handler->setGlobalValues(gValues);
                dataPos += sizeof(ClientControlGlia);
                break;
            }

            case CLIENT_CONTROL_OUTPUT_LEARNING:
            {
                LOG_DEBUG("CLIENT_CONTROL_OUTPUT_LEARNING");
                const ClientControlOutputLearning content
                        = *((ClientControlOutputLearning*)&dataObj[dataPos]);
                Brick* brick = rootObject->m_brickHandler->getBrick(content.brickId);
                assert(brick != nullptr);

                brick->learningOverride = content.outputLearning;

                dataPos += sizeof(ClientControlOutputLearning);
                break;
            }

            case CLIENT_LEARN_INPUT:
            {
                LOG_DEBUG("CLIENT_LEARN_INPUT");
                const ClientLearnInputData content = *((ClientLearnInputData*)&dataObj[dataPos]);
                Brick* brick = rootObject->m_brickHandler->getBrick(content.brickId);
                assert(brick != nullptr);

                Kitsunemimi::StackBuffer* newBuffer = new StackBuffer();
                const uint16_t ok = brick->dataConnections[NODE_DATA].inUse > 0;

                for(uint16_t i = 0; i < ok * NUMBER_OF_NODES_PER_BRICK; i++)
                {
                    DirectEdgeContainer newEdge;
                    newEdge.weight = content.value;
                    newEdge.targetNodeId = i;
                    addObjectToBuffer(*newBuffer, &newEdge);
                }

                finishSide(*brick, 24);
                dataPos += sizeof(ClientLearnInputData);
                break;
            }

            default:
                assert(false);
                break;
        }
    }
}

/**
 * @brief standaloneDataCallback
 * @param target
 * @param data
 * @param dataSize
 */
void
standaloneDataCallback(void* target,
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
void
errorCallback(void*,
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
void
sessionCallback(void* target,
                bool isInit,
                Kitsunemimi::Project::Session* session,
                const std::string sessionIdentifier)
{
    LOG_INFO("register incoming session with identifier: " + sessionIdentifier);

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

}

#endif // NETWORK_CALLBACKS_H
