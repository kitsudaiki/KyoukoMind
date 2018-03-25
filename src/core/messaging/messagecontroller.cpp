#include "messagecontroller.h"
#include <core/messaging/messages/datamessage.h>
#include <core/messaging/messages/replymessage.h>
#include <core/messaging/messages/learningmessage.h>
#include <core/messaging/messages/learningreplymessage.h>

namespace KyoukoMind
{

MessageController::MessageController()
{
    //m_networkConnection = new NetworkConnection::NetworkConnection();
}

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
        default:
            break;
    }
    return nullptr;
}

}
