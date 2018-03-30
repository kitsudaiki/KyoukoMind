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
        m_mutex.lock();
        if(message->getType() == CYCLEFINISHMESSAGE) {
            m_finishCounter++;
            if(isFinished()) {
                m_switchFlag = !m_switchFlag;
                m_mutex.unlock();
                return true;
            }
        } else {
            if(m_switchFlag) {
                m_messageQueue1[site].push_back(message);
            } else {
                m_messageQueue2[site].push_back(message);
            }
        }
        m_mutex.unlock();
        return true;
    }
    return false;
}

/**
 * @brief IncomingMessageQueue::getMessageQueue
 * @return
 */
std::vector<Message *> *IncomingMessageQueue::getMessageQueue(const uint8_t site)
{
    if(m_switchFlag) {
        return &m_messageQueue1[site];
    } else {
        return &m_messageQueue2[site];
    }
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

/**
 * @brief IncomingMessageQueue::resetFinishCounter
 */
void IncomingMessageQueue::resetFinishCounter()
{
    m_finishCounter = 0;
}

}
