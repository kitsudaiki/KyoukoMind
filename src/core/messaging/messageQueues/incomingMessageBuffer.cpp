#include "incomingMessageBuffer.h"

#include <core/messaging/messages/message.h>
#include <core/messaging/messagecontroller.h>

namespace KyoukoMind
{

/**
 * @brief IncomingMessageBuffer::IncomingMessageBuffer
 * @param clusterId
 * @param controller
 */
IncomingMessageBuffer::IncomingMessageBuffer(const ClusterID clusterId,
                                             MessageController* controller):
    MessageBuffer(clusterId, controller)
{
    controller->addIncomingMessageQueue(clusterId, this);
}

/**
 * @brief IncomingMessageBuffer::addMessage
 * @param site
 * @param message
 * @return
 */
bool IncomingMessageBuffer::addMessage(const uint8_t site, Message *message)
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
 * @brief IncomingMessageBuffer::getMessageQueue
 * @return
 */
std::vector<Message *> *IncomingMessageBuffer::getMessageQueue(const uint8_t site)
{
    if(m_switchFlag) {
        return &m_messageQueue1[site];
    } else {
        return &m_messageQueue2[site];
    }
}

/**
 * @brief IncomingMessageBuffer::isFinished
 * @return
 */
bool IncomingMessageBuffer::isFinished() const
{
    if(m_finishCounter == 9) {
        return true;
    }
    return false;
}

/**
 * @brief IncomingMessageBuffer::resetFinishCounter
 */
void IncomingMessageBuffer::resetFinishCounter()
{
    m_finishCounter = 0;
}

}
