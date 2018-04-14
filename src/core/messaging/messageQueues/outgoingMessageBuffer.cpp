/**
 *  @file    outgoingMessageBuffer.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "outgoingMessageBuffer.h"

#include <core/messaging/messageController.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messages/dataAxonMessage.h>
#include <core/messaging/messages/replyMessage.h>
#include <core/messaging/messages/learningMessage.h>
#include <core/messaging/messages/learningReplyMessage.h>
#include <core/messaging/messages/cycleFinishMessage.h>

namespace KyoukoMind
{

/**
 * @brief OutgoingMessageBuffer::OutgoingMessageBuffer
 * @param clusterId
 * @param controller
 */
OutgoingMessageBuffer::OutgoingMessageBuffer(const ClusterID clusterId,
                                             MessageController* controller):
    MessageBuffer(clusterId, controller)
{
    for(uint32_t i = 0; i < 16; i++) {
        m_dataMessageBuffer[i] = new DataMessage();
        m_dataAxonMessageBuffer[i] = new DataAxonMessage();
        m_learingMessageBuffer[i] = new LearningMessage();
        m_learingReplyMessageBuffer[i] = new LearningReplyMessage();
    }
}

/**
 * @brief OutgoingMessageBuffer::addEdge
 * @param targetClusterId
 * @param targetSite
 * @param newEdge
 * @return
 */
bool OutgoingMessageBuffer::addEdge(const ClusterID targetClusterId,
                                    const uint8_t targetSite,
                                    const KyoChanEdge newEdge)
{
    if(targetSite < 16) {
        if(m_dataMessageBuffer[targetSite]->addEdge(newEdge)) {
            return true;
        }
        m_dataMessageBuffer[targetSite]->setMetaData(targetClusterId,
                                                     m_clusterId,
                                                     m_messageIdCounter,
                                                     targetSite);
        m_controller->sendMessage(m_dataMessageBuffer[targetSite]);
        m_dataMessageBuffer[targetSite] = new DataMessage();
        m_messageIdCounter++;
        m_dataMessageBuffer[targetSite]->addEdge(newEdge);
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
bool OutgoingMessageBuffer::addAxonEdge(const ClusterID targetClusterId,
                                        const uint8_t targetSite,
                                        const KyoChanAxonEdge newAxonEdge)
{
    if(targetSite < 16) {
        if(m_dataAxonMessageBuffer[targetSite]->addAxonEdge(newAxonEdge)) {
            return true;
        }
        m_dataAxonMessageBuffer[targetSite]->setMetaData(targetClusterId,
                                                         m_clusterId,
                                                         m_messageIdCounter,
                                                         targetSite);
        m_controller->sendMessage(m_dataMessageBuffer[targetSite]);
        m_dataAxonMessageBuffer[targetSite] = new DataAxonMessage();
        m_messageIdCounter++;
        m_dataAxonMessageBuffer[targetSite]->addAxonEdge(newAxonEdge);
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
bool OutgoingMessageBuffer::addLearingEdge(const ClusterID targetClusterId,
                                           const uint8_t targetSite,
                                           const KyoChanNewEdge newEdge)
{
    if(targetSite <= 9) {
        if(m_learingMessageBuffer[targetSite]->addNewEdge(newEdge)) {
            return true;
        }
        m_learingMessageBuffer[targetSite]->setMetaData(targetClusterId,
                                                        m_clusterId,
                                                        m_messageIdCounter,
                                                        targetSite);
        m_controller->sendMessage(m_learingMessageBuffer[targetSite]);
        m_learingMessageBuffer[targetSite] = new LearningMessage();
        m_messageIdCounter++;
        m_learingMessageBuffer[targetSite]->addNewEdge(newEdge);
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::addLearningReplyMessage
 * @param targetClusterId
 * @param targetSite
 * @param newEdgeReply
 */
bool OutgoingMessageBuffer::addLearningReplyMessage(const ClusterID targetClusterId,
                                                    const uint8_t targetSite,
                                                    const KyoChanNewEdgeReply newEdgeReply)
{
    if(targetSite <= 9) {
        if(m_learingReplyMessageBuffer[targetSite]->addNewEdgeReply(newEdgeReply)) {
            return true;
        }
        m_learingReplyMessageBuffer[targetSite]->setMetaData(targetClusterId,
                                                             m_clusterId,
                                                             m_messageIdCounter,
                                                             targetSite);
        m_controller->sendMessage(m_learingReplyMessageBuffer[targetSite]);
        m_learingReplyMessageBuffer[targetSite] = new LearningReplyMessage();
        m_messageIdCounter++;
        m_learingReplyMessageBuffer[targetSite]->addNewEdgeReply(newEdgeReply);
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::sendReplyMessage
 * @param targetClusterId
 * @param targetSite
 */
void OutgoingMessageBuffer::sendReplyMessage(const ClusterID targetClusterId,
                                             const uint8_t targetSite)
{
    ReplyMessage* replyMessage = new ReplyMessage(targetClusterId,
                                                  m_messageIdCounter,
                                                  targetSite);
    m_controller->sendMessage(replyMessage);
    m_messageIdCounter++;
}

/**
 * @brief OutgoingMessageBuffer::sendFinishCycle
 * @param targetClusterId
 * @param targetSite
 */
void OutgoingMessageBuffer::sendFinishCycle(const ClusterID targetClusterId,
                                            const uint8_t targetSite)
{
    m_dataMessageBuffer[targetSite]->setMetaData(targetClusterId,
                                                 m_clusterId,
                                                 m_messageIdCounter,
                                                 targetSite);
    m_controller->sendMessage(m_dataMessageBuffer[targetSite]);
    m_dataMessageBuffer[targetSite] = new DataMessage();
    m_messageIdCounter++;

    CycleFinishMessage* finishMessage = new CycleFinishMessage(targetClusterId,
                                                               m_messageIdCounter,
                                                               targetSite);
    m_controller->sendMessage(finishMessage);
    m_messageIdCounter++;
}

}
