/**
 *  @file    incomingMessageBuffer.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "incomingMessageBuffer.h"

#include <core/messaging/messages/message.h>
#include <core/messaging/messageController.h>

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
 * @param side
 * @param message
 * @return
 */
bool IncomingMessageBuffer::addMessage(const uint8_t side, Message *message)
{
    if(side <= 15) {
        m_mutex.lock();
        if(message->getType() == CYCLE_FINISH_MESSAGE) {
            m_finishCounter++;
            if(isReady()) {
                m_switchFlag = !m_switchFlag;
                m_mutex.unlock();
                return true;
            }
        } else {
            if(m_switchFlag) {
                m_messageQueue1[side].push_back(message);
            } else {
                m_messageQueue2[side].push_back(message);
            }
        }
        m_mutex.unlock();
        return true;
    }
    return false;
}

/**
 * @brief IncomingMessageBuffer::getMessageQueue
 * @param side
 * @return
 */
std::vector<Message *> *IncomingMessageBuffer::getMessageQueue(const uint8_t side)
{
    if(m_switchFlag) {
        return &m_messageQueue1[side];
    } else {
        return &m_messageQueue2[side];
    }
}

/**
 * @brief IncomingMessageBuffer::isFinished
 * @return
 */
bool IncomingMessageBuffer::isReady() const
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
