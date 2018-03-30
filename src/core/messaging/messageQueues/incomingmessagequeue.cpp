#include "incomingmessagequeue.h"

#include <core/messaging/messages/message.h>

namespace KyoukoMind
{

/**
 * @brief IncomingMessageQueue::IncomingMessageQueue
 * @param clusterId
 * @param controller
 */
IncomingMessageQueue::IncomingMessageQueue(const ClusterID clusterId,
                                           MessageController* controller):
    MessageQueue(clusterId, controller)
{

}

/**
 * @brief IncomingMessageQueue::addMessage
 * @param site
 * @param message
 * @return
 */
bool IncomingMessageQueue::addMessage(const uint8_t site, Message *message)
{
    if(site <= 9) {
        if(message->getType() == CYCLEFINISHMESSAGE) {
            m_finishCounter++;
        } else {
            m_messages[site].push_back(message);
        }
        return true;
    }
    return false;
}

/**
 * @brief IncomingMessageQueue::isFinished
 * @return
 */
bool IncomingMessageQueue::isFinished() const
{
    if(m_finishCounter == 9) {
        return true;
    }
    return false;
}

}
