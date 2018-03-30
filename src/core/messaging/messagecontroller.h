#ifndef MESSAGECONTROLLER_H
#define MESSAGECONTROLLER_H

#include <common.h>
#include <core/messaging/messages/message.h>

namespace KyoukoMind
{
class Message;
class DataMessage;
class ReplyMessage;
class LearningMessage;
class LearningReployMessage;
class CycleFinishMessage;

class IncomingMessageQueue;

class MessageController
{
public:
    MessageController();

    bool addIncomingMessageQueue(const uint32_t clusterId,
                                 IncomingMessageQueue *messageQueue);

    bool sendMessage(Message* message);

private:
    Message* convertToMessage(uint8_t *data);
    std::map<uint32_t, IncomingMessageQueue*> m_messageQueues;
};

}

#endif // MESSAGECONTROLLER_H
