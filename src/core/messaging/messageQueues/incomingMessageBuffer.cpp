/**
 *  @file    incomingMessageBuffer.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "incomingMessageBuffer.h"

#include <core/clustering/cluster/cluster.h>
#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messageController.h>

namespace KyoukoMind
{

/**
 * @brief IncomingMessageBuffer::IncomingMessageBuffer
 * @param clusterId
 * @param controller
 */
IncomingMessageBuffer::IncomingMessageBuffer(Cluster *cluster,
                                             MessageController* controller):
    MessageBuffer(cluster, controller)
{
    controller->addIncomingMessageQueue(cluster->getClusterId(), this);
    initMessageBuffer(cluster);
}

/**
 * @brief IncomingMessageBuffer::initMessageBuffer
 * @param cluster
 * @return
 */
bool IncomingMessageBuffer::initMessageBuffer(Cluster *cluster)
{
    for(uint32_t side = 0; side < 17; side++)
    {
        m_buffer[side].m_dataMessageBuffer1 = new DataMessage(cluster->getNeighborId(side),
                                                     0,
                                                     16 - side,
                                                     1);
        m_buffer[side].m_dataMessageBuffer2 = new DataMessage(cluster->getNeighborId(side),
                                                     0,
                                                     16 - side,
                                                     1);
    }
}

/**
 * @brief IncomingMessageBuffer::addMessage
 * @param side
 * @param message
 * @return
 */
bool IncomingMessageBuffer::addMessage(const uint8_t side, DataMessage *message)
{
    if(side < 17)
    {
        m_mutex.lock();
        if(m_buffer[side].switchFlag) {
            m_buffer[side].m_dataMessageBuffer1 = message;
        } else {
            m_buffer[side].m_dataMessageBuffer2 = message;
        }
        m_buffer[side].switchFlag = !m_buffer[side].switchFlag;
        m_mutex.unlock();
        return true;
    }
    return false;
}

/**
 * @brief IncomingMessageBuffer::getMessage
 * @param side
 * @return
 */
Message *IncomingMessageBuffer::getMessage(const uint8_t side)
{
    if(!m_buffer[side].switchFlag) {
        DataMessage* message = m_buffer[side].m_dataMessageBuffer1;
        return message;
    } else {
        DataMessage* message = m_buffer[side].m_dataMessageBuffer2;
        return message;
    }
}

/**
 * @brief IncomingMessageBuffer::isFinished
 * @return
 */
bool IncomingMessageBuffer::isReady() const
{
    return true;
    // TODO
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
