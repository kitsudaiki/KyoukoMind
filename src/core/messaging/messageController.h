/**
 *  @file    messageController.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef MESSAGECONTROLLER_H
#define MESSAGECONTROLLER_H

#include <common.h>
#include <core/messaging/messages/message.h>

namespace KyoukoMind
{
class Message;
class DataMessage;
class DataAxonMessage;
class ReplyMessage;
class LearningMessage;
class LearningReplyMessage;
class CycleFinishMessage;

class IncomingMessageBuffer;

class MessageController
{
public:
    MessageController();

    bool addIncomingMessageQueue(const uint32_t clusterId,
                                 IncomingMessageBuffer *messageQueue);

    bool sendMessage(Message* message);

private:
    Message* convertToMessage(uint8_t *data, uint32_t size);
    std::map<uint32_t, IncomingMessageBuffer*> m_messageQueues;
};

}

#endif // MESSAGECONTROLLER_H
