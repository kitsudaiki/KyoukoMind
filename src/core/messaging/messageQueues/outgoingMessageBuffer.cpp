/**
 *  @file    outgoingMessageBuffer.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "outgoingMessageBuffer.h"

#include <core/clustering/cluster/cluster.h>
#include <core/messaging/messageController.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messages/replyMessage.h>

namespace KyoukoMind
{

/**
 * @brief OutgoingMessageBuffer::OutgoingMessageBuffer
 * @param clusterId
 * @param controller
 */
OutgoingMessageBuffer::OutgoingMessageBuffer(Cluster* cluster,
                                             MessageController* controller):
    MessageBuffer(cluster, controller)
{
    for(uint32_t side = 0; side < 16; side++)
    {
        m_dataMessageBuffer[side] = nullptr;
    }
    updateBufferInit();
}

/**
 * @brief OutgoingMessageBuffer::updateBufferInit
 * @return
 */
bool OutgoingMessageBuffer::updateBufferInit()
{
    for(uint32_t side = 0; side < 16; side++)
    {
        if(m_dataMessageBuffer[side] != nullptr) {
            delete m_dataMessageBuffer[side];
        }

        m_dataMessageBuffer[side] = new DataMessage(m_cluster->getNeighborId(side),
                                                    m_cluster->getClusterId(),
                                                    15 - side);
    }
}

/**
 * @brief OutgoingMessageBuffer::addEdge
 * @param targetSite
 * @param newEdge
 * @return
 */
bool OutgoingMessageBuffer::addEdge(const uint8_t sourceSite,
                                    const KyoChanEdgeContainer *edge)
{
    if(sourceSite < 16) {
        m_dataMessageBuffer[sourceSite]->addEdge(edge);
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::addPendingEdge
 * @param sourceSite
 * @param edge
 * @return
 */
bool OutgoingMessageBuffer::addPendingEdge(const uint8_t sourceSite,
                                           const KyoChanPendingEdgeContainer *edge)
{
    if(sourceSite < 16) {
        m_dataMessageBuffer[sourceSite]->addPendingEdge(edge);
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::addAxonEdge
 * @param targetClusterId
 * @param targetSite
 * @param newAxonEdge
 * @return
 */
bool OutgoingMessageBuffer::addAxonEdge(const uint8_t sourceSite,
                                        const KyoChanAxonEdgeContainer *newAxonEdge)
{
    if(sourceSite < 16) {
        m_dataMessageBuffer[sourceSite]->addAxonEdge(newAxonEdge);
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::addLearingEdge
 * @param targetClusterId
 * @param targetSite
 * @return
 */
bool OutgoingMessageBuffer::addLearingEdge(const uint8_t sourceSite,
                                           const KyoChanLearingEdgeContainer *newEdge)
{
    if(sourceSite <= 16) {
        m_dataMessageBuffer[sourceSite]->addNewEdge(newEdge);
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::addLearningReplyMessage
 * @param targetClusterId
 * @param newEdgeReply
 */
bool OutgoingMessageBuffer::addLearningReplyMessage(const uint8_t sourceSite,
                                                    const KyoChanLearningEdgeReplyContainer *newEdgeReply)
{
    if(sourceSite <= 16) {
        m_dataMessageBuffer[sourceSite]->addNewEdgeReply(newEdgeReply);
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::sendReplyMessage
 * @param sourceSite
 */
void OutgoingMessageBuffer::sendReplyMessage(const uint8_t sourceSite)
{
    ReplyMessage* replyMessage = new ReplyMessage(m_cluster->getNeighborId(sourceSite),
                                                  15 - sourceSite);
    m_controller->sendMessage(replyMessage);
    m_messageIdCounter++;
}

/**
 * @brief OutgoingMessageBuffer::sendFinishCycle
 * @param sourceSite
 */
void OutgoingMessageBuffer::finishCycle(const uint8_t sourceSite)
{
    m_controller->sendMessage(m_dataMessageBuffer[sourceSite]);
    m_dataMessageBuffer[sourceSite] = new DataMessage(m_cluster->getNeighborId(sourceSite),
                                                      m_cluster->getClusterId(),
                                                      15 - sourceSite);
}

}
