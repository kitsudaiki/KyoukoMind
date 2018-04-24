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
    for(uint32_t side = 0; side < 16; side++)
    {
        m_dataMessageBuffer1[side] = new DataMessage(cluster->getNeighborId(side),
                                                     cluster->getClusterId(),
                                                     15 - side);
        m_dataMessageBuffer2[side] = new DataMessage(cluster->getNeighborId(side),
                                                     cluster->getClusterId(),
                                                     15 - side);
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
    std::cout<<"   add Message  side: "<<(int)side<<"   payload: "<<(int)message->getPayloadSize()<<std::endl;
    if(side <= 15)
    {
        m_mutex.lock();
        if(m_switchFlag) {
            m_dataMessageBuffer1[side] = message;
        } else {
            m_dataMessageBuffer2[side] = message;
        }
        m_finishCounter++;
        if(isReady()) {
            m_switchFlag = !m_switchFlag;
        }
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
    if(!m_switchFlag) {
        DataMessage* message = m_dataMessageBuffer1[side];
        return message;
    } else {
        DataMessage* message = m_dataMessageBuffer2[side];
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
