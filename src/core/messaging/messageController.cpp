/**
 *  @file    messageHandler.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "messageController.h"

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messages/replyMessage.h>
#include <core/messaging/messages/learningMessage.h>
#include <core/messaging/messages/learningReplyMessage.h>
#include <core/messaging/messages/cycleFinishMessage.h>

#include <core/messaging/messageQueues/incomingMessageBuffer.h>

namespace KyoukoMind
{

/**
 * @brief MessageController::MessageController
 */
MessageController::MessageController() {}

/**
 * @brief MessageController::addIncomingMessageQueues
 * @param clusterId
 * @param messageQueue
 * @return
 */
bool MessageController::addIncomingMessageQueue(const uint32_t clusterId,
                                                IncomingMessageBuffer *messageQueue)
{
    if(m_messageQueues.find(clusterId) == m_messageQueues.end()) {
        // TODO: check if insert was successful
        m_messageQueues.insert(std::make_pair(clusterId, messageQueue));
        return true;
    }
    return false;
}

/**
 * @brief MessageController::sendMessage
 * @param message
 * @return
 */
bool MessageController::sendMessage(Message *message)
{
    uint32_t targetClusterId = message->getMetaData().targetClusterId;
    uint8_t targetSite = message->getMetaData().targetSite;

    std::map<ClusterID, IncomingMessageBuffer*>::iterator it;
    it = m_messageQueues.find(targetClusterId);
    if(it != m_messageQueues.end()) {
        IncomingMessageBuffer* targetQueue = it->second;
        targetQueue->addMessage(targetSite, message);
        return true;
    }
    return false;
}

/**
 * @brief MessageController::convertToMessage
 * @param data
 * @return
 */
Message* MessageController::convertToMessage(uint8_t *data)
{
    if(data == nullptr) {
        return nullptr;
    }
    uint8_t type = data[0];
    switch(type) {
        case UNDEFINED:
            break;
        case DATAMESSAGE:
            {
                DataMessage* message = new DataMessage();
                message->convertFromByteArray(data);
                delete data;
                return message;
            }
        case REPLYMESSAGE:
            {
                ReplyMessage* message = new ReplyMessage();
                message->convertFromByteArray(data);
                delete data;
                return message;
            }
        case LEARNINGMESSAGE:
            {
                LearningMessage* message = new LearningMessage();
                message->convertFromByteArray(data);
                delete data;
                return message;
            }
        case LEARNINGREPLYMESSAGE:
            {
                LearningReplyMessage* message = new LearningReplyMessage();
                message->convertFromByteArray(data);
                delete data;
                return message;
            }
        case CYCLEFINISHMESSAGE:
            {
                CycleFinishMessage* message = new CycleFinishMessage();
                message->convertFromByteArray(data);
                delete data;
                return message;
            }
        default:
            break;
    }
    return nullptr;
}

}
