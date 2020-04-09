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
using Kitsunemimi::Kyouko::ControlDoesBrickExist;
using Kitsunemimi::Kyouko::ControlDoesBrickExistResponse;
using Kitsunemimi::Kyouko::ControlGetMetadata;
using Kitsunemimi::Kyouko::ControlGetMetadataResponse;

namespace KyoukoMind
{

/**
 * @brief send_generic_response
 * @param success
 * @param errorMessage
 * @param session
 * @param blockerId
 */
void
send_generic_response(const bool success,
                      const std::string &errorMessage,
                      Kitsunemimi::Project::Session* session,
                      const uint64_t blockerId)
{

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

/**
 * @brief send_doesBrickExist_response
 * @param success
 * @param errorMessage
 * @param session
 * @param blockerId
 */
void
send_doesBrickExist_response(const bool result,
                             Kitsunemimi::Project::Session* session,
                             const uint64_t blockerId)
{
    ControlDoesBrickExistResponse response;
    response.result = result;

    session->sendResponse(&response, sizeof(ControlDoesBrickExistResponse), blockerId);
}

/**
 * @brief send_metadata_response
 * @param metadata
 * @param session
 * @param blockerId
 */
void
send_metadata_response(const BrickHandler::Metadata metadata,
                       Kitsunemimi::Project::Session* session,
                       const uint64_t blockerId)
{
    ControlGetMetadataResponse response;
    response.numberOfEdgeBricks = metadata.numberOfEdgeBricks;
    response.numberOfNodeBricks = metadata.numberOfNodeBricks;
    response.numberOfNodesPerBrick = NUMBER_OF_NODES_PER_BRICK;

    session->sendResponse(&response, sizeof(ControlGetMetadataResponse), blockerId);
}

/**
 * @brief registerInput
 * @param content
 * @param errorMessage
 * @return
 */
bool
process_registerInput(const ControlRegisterInput &content,
                      RootObject* rootObject,
                      Kitsunemimi::Project::Session* session,
                      const uint64_t blockerId)
{
    std::string errorMessage = "";
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
        send_generic_response(false, errorMessage, session, blockerId);
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
        send_generic_response(false, errorMessage, session, blockerId);
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
        send_generic_response(false, errorMessage, session, blockerId);
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
        send_generic_response(false, errorMessage, session, blockerId);
        return false;
    }
    rootObject->m_inputBricks->insert(std::pair<uint32_t, Brick*>(content.brickId,
                                                                  newBrick));
    initCycle(newBrick);

    send_generic_response(true, "", session, blockerId);

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
process_registerOutput(const ControlRegisterOutput &content,
                       RootObject* rootObject,
                       Kitsunemimi::Project::Session* session,
                       const uint64_t blockerId)
{
    std::string errorMessage = "";

    // check if target-brick, which is specified by the id in the messge, does exist
    Brick* outgoingBrick = rootObject->m_brickHandler->getBrick(content.brickId);
    if(outgoingBrick == nullptr)
    {
        errorMessage = "register output failed: brick with id "
                       + std::to_string(content.brickId)
                       + " doesn't exist.";
        LOG_ERROR(errorMessage);
        send_generic_response(false, errorMessage, session, blockerId);
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
        send_generic_response(false, errorMessage, session, blockerId);
        return false;
    }

    addClientOutputConnection(*outgoingBrick);
    send_generic_response(true, "", session, blockerId);

    return true;
}

/**
 * @brief doesBrickExist
 * @param content
 * @param rootObject
 * @return
 */
bool
process_doesBrickExist(const ControlDoesBrickExist &content,
                       RootObject* rootObject,
                       Kitsunemimi::Project::Session* session,
                       const uint64_t blockerId)
{
    Brick* outgoingBrick = rootObject->m_brickHandler->getBrick(content.brickId);
    const bool exist = outgoingBrick != nullptr;
    send_doesBrickExist_response(exist, session, blockerId);
    return exist;
}

/**
 * @brief process_getMetadata
 * @param content
 * @param rootObject
 * @param session
 * @param blockerId
 */
void
process_getMetadata(RootObject* rootObject,
                    Kitsunemimi::Project::Session* session,
                    const uint64_t blockerId)
{
    const BrickHandler::Metadata metadata = rootObject->m_brickHandler->getMetadata();
    send_metadata_response(metadata, session, blockerId);
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

    LOG_DEBUG("process incoming client message with size: " + std::to_string(data->bufferPosition));

    const uint8_t* dataObj = static_cast<const uint8_t*>(data->data);
    const uint8_t type = dataObj[0];

    switch(type)
    {
        case CONTROL_REGISTER_INPUT:
        {
            LOG_DEBUG("CONTROL_REGISTER_INPUT");
            assert(sizeof(ControlRegisterInput) == data->bufferPosition);
            const ControlRegisterInput content
                    = *(static_cast<const ControlRegisterInput*>(data->data));
            process_registerInput(content, rootObject, session, blockerId);
            break;
        }

        case CONTROL_REGISTER_OUTPUT:
        {
            LOG_DEBUG("CONTROL_REGISTER_OUTPUT");
            assert(sizeof(ControlRegisterOutput) == data->bufferPosition);
            const ControlRegisterOutput content
                    = *(static_cast<const ControlRegisterOutput*>(data->data));
            process_registerOutput(content, rootObject, session, blockerId);
            break;
        }

        case CONTROL_DOES_BRICK_EXIST:
        {
            LOG_DEBUG("CONTROL_DOES_BRICK_EXIST");
            assert(sizeof(ControlDoesBrickExist) == data->bufferPosition);
            const ControlDoesBrickExist content
                    = *(static_cast<const ControlDoesBrickExist*>(data->data));
            process_doesBrickExist(content, rootObject, session, blockerId);
            break;
        }

        case CONTROL_GET_METADATA:
        {
            LOG_DEBUG("CONTROL_GET_METADATA");
            assert(sizeof(ControlGetMetadata) == data->bufferPosition);
            process_getMetadata(rootObject, session, blockerId);
            break;
        }

        default:
            assert(false);
            break;
    }

    delete data;
}

}

#endif // CONTROL_PROCESSING_H
