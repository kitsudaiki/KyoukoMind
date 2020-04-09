#ifndef CONTROL_PROCESSING_H
#define CONTROL_PROCESSING_H

#include <root_object.h>
#include <core/objects/container_definitions.h>
#include <core/processing/processing_methods/brick_initializing_methods.h>
#include <core/processing/processing_methods/brick_processing_methods.h>
#include <core/processing/processing_methods/neighbor_methods.h>

#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>
#include <libKitsunemimiPersistence/logger/logger.h>

#include <libKitsunemimiKyoukoCommon/communication_structs/control_contianer.h>

using Kitsunemimi::Kyouko::ControlRegisterInput;
using Kitsunemimi::Kyouko::ControlRegisterOutput;
using Kitsunemimi::Kyouko::ControlResponse;

namespace KyoukoMind
{

/**
 * @brief registerInput
 * @param content
 * @param errorMessage
 * @return
 */
bool
registerInput(const ControlRegisterInput &content,
              RootObject* rootObject,
              std::string &errorMessage)
{
    const uint32_t fakeId = 10000 + content.brickId;
    const uint8_t sourceSide = 22;

    // check if target-brick, which is specified by the id in the messge, does exist
    Brick* targetBrick = rootObject->m_brickHandler->getBrick(content.brickId);
    if(targetBrick == nullptr)
    {
        errorMessage = "register input failed: brick with id "
                       + std::to_string(content.brickId)
                       + " doesn't exist.";
        LOG_ERROR(errorMessage);
        return false;
    }

    // check if brick is node-brick
    const uint8_t isNodeBrick = targetBrick->dataConnections[NODE_DATA].inUse;
    if(isNodeBrick == 0)
    {
        errorMessage = "register input failed: brick with id "
                       + std::to_string(content.brickId)
                       + " is not a node-brick.";
        LOG_ERROR(errorMessage);
        return false;
    }

    // check if brick is already registerd as input
    std::map<uint32_t, Brick*>::const_iterator it;
    it = rootObject->m_inputBricks->find(content.brickId);
    if(it != rootObject->m_inputBricks->end())
    {
        errorMessage = "register input failed: brick with id "
                       + std::to_string(content.brickId)
                       + " is already registered.";
        LOG_ERROR(errorMessage);
        return false;
    }

    // create fake-brick to connect it with the defined input-brick
    Brick* newBrick = new Brick(fakeId, 0, 0);
    newBrick->isInputBrick = 1;

    // init the new neighbors
    const bool success = connectBricks(*newBrick, sourceSide, *targetBrick);
    if(success == false)
    {
        delete newBrick;
        errorMessage = "register input failed: brick with id "
                       + std::to_string(content.brickId)
                       + " can not be prepared for an unknown reason.";
        LOG_ERROR(errorMessage);
        return false;
    }
    rootObject->m_inputBricks->insert(std::pair<uint32_t, Brick*>(content.brickId,
                                                                  newBrick));
    initCycle(newBrick);

    return true;
}

/**
 * @brief registerOutput
 * @param content
 * @param rootObject
 * @param errorMessage
 * @return
 */
bool
registerOutput(const ControlRegisterOutput &content,
               RootObject* rootObject,
               std::string &errorMessage)
{
    // check if target-brick, which is specified by the id in the messge, does exist
    Brick* outgoingBrick = rootObject->m_brickHandler->getBrick(content.brickId);
    if(outgoingBrick == nullptr)
    {
        errorMessage = "register output failed: brick with id "
                       + std::to_string(content.brickId)
                       + " doesn't exist.";
        LOG_ERROR(errorMessage);
        return false;
    }

    // check if brick is node-brick
    const uint8_t isNodeBrick = outgoingBrick->dataConnections[NODE_DATA].inUse;
    if(isNodeBrick == 0)
    {
        errorMessage = "register output failed: brick with id "
                       + std::to_string(content.brickId)
                       + " is not a node-brick.";
        LOG_ERROR(errorMessage);
        return false;
    }

    addClientOutputConnection(*outgoingBrick);
}

/**
 * @brief controlCallback
 * @param target
 * @param session
 * @param blockerId
 * @param data
 */
void
controlCallback(void* target,
                Kitsunemimi::Project::Session* session,
                const uint64_t blockerId,
                DataBuffer* data)
{
    RootObject* rootObject = static_cast<RootObject*>(target);
    const uint8_t* dataObj = static_cast<const uint8_t*>(data->data);

    LOG_DEBUG("process incoming client message with size: " + std::to_string(data->bufferPosition));

    bool success = true;
    std::string errorMessage = "";

    uint64_t dataPos = 0;
    while(dataPos < data->bufferPosition)
    {
        if(success == false) {
            break;
        }
        const uint8_t type = dataObj[dataPos];

        switch(type)
        {
            case CONTROL_REGISTER_INPUT:
            {
                LOG_DEBUG("CONTROL_REGISTER_INPUT");
                const ControlRegisterInput content = *((ControlRegisterInput*)&dataObj[dataPos]);
                success = registerInput(content, rootObject, errorMessage);
                dataPos += sizeof(ControlRegisterInput);
                break;
            }

            case CONTROL_REGISTER_OUTPUT:
            {
                LOG_DEBUG("CONTROL_REGISTER_OUTPUT");
                const ControlRegisterOutput content = *((ControlRegisterOutput*)&dataObj[dataPos]);
                success = registerOutput(content, rootObject, errorMessage);
                dataPos += sizeof(ControlRegisterOutput);
                break;
            }

            default:
                assert(false);
                break;
        }
    }
    delete data;

    // build and send response message
    ControlResponse response;
    response.success = success;
    if(success == false)
    {
        strncpy(response.errorMessage, errorMessage.c_str(), errorMessage.size());
        response.messageSize = errorMessage.size();
    }
    session->sendResponse(&response, sizeof(ControlResponse), blockerId);
}

}

#endif // CONTROL_PROCESSING_H
