#ifndef MESSAGECONTROLLER_H
#define MESSAGECONTROLLER_H

#include <common.h>
#include <core/messaging/messages/message.h>

class DataMessage;
class ReplyMessage;
class LearningMessage;
class LearningReployMessage;

namespace KyoukoMind
{

class MessageController
{
public:
    MessageController();

private:
    Message* convertToMessage(uint8_t *data);
    //NetworkConnection::NetworkConnection *m_networkConnection = nullptr;
};

}

#endif // MESSAGECONTROLLER_H
