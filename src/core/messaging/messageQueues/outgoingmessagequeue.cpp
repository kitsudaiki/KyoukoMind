#include "outgoingmessagequeue.h"

#include <core/messaging/messagecontroller.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/datamessage.h>
#include <core/messaging/messages/replymessage.h>
#include <core/messaging/messages/learningmessage.h>
#include <core/messaging/messages/learningreplymessage.h>
#include <core/messaging/messages/cyclefinishmessage.h>

namespace KyoukoMind
{

/**
 * @brief OutgoingMessageQueue::OutgoingMessageQueue
 * @param clusterId
 * @param controller
 */
OutgoingMessageQueue::OutgoingMessageQueue(const ClusterID clusterId,
                                           MessageController* controller):
    MessageQueue(clusterId, controller)
{
    for(unsigned int i = 0; i < 10; i++) {
        m_dataMessageBuffer[i] = new DataMessage();
        m_learingMessageBuffer[i] = new LearningMessage();
    }
}

/**
 * @brief OutgoingMessageQueue::addEdge
 * @param targetClusterId
 * @param targetSite
 * @param newEdge
 * @return
 */
bool OutgoingMessageQueue::addEdge(const ClusterID targetClusterId,
                                   const uint8_t targetSite,
                                   const KyoChanEdge newEdge)
{
    if(targetSite <= 9) {
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
 * @brief OutgoingMessageQueue::addLearingEdge
 * @param targetClusterId
 * @param targetSite
 * @return
 */
bool OutgoingMessageQueue::addLearingEdge(const ClusterID targetClusterId,
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
 * @brief OutgoingMessageQueue::sendReplyMessage
 * @param targetClusterId
 * @param targetSite
 */
void OutgoingMessageQueue::sendReplyMessage(const ClusterID targetClusterId,
                                            const uint8_t targetSite)
{
    ReplyMessage* replyMessage = new ReplyMessage(targetClusterId,
                                                  m_messageIdCounter,
                                                  targetSite);
    m_controller->sendMessage(replyMessage);
    m_messageIdCounter++;
}

/**
 * @brief OutgoingMessageQueue::sendLearningReplyMessage
 * @param targetClusterId
 * @param targetSite
 */
void OutgoingMessageQueue::sendLearningReplyMessage(const ClusterID targetClusterId,
                                                    const uint8_t targetSite)
{
    LearningReplyMessage* learningReplyMessage = new LearningReplyMessage(targetClusterId,
                                                                          m_messageIdCounter,
                                                                          targetSite);
    m_controller->sendMessage(learningReplyMessage);
    m_messageIdCounter++;
}

/**
 * @brief OutgoingMessageQueue::sendFinishCycle
 * @param targetClusterId
 * @param targetSite
 */
void OutgoingMessageQueue::sendFinishCycle(const ClusterID targetClusterId,
                                           const uint8_t targetSite)
{
    CycleFinishMessage* finishMessage = new CycleFinishMessage(targetClusterId,
                                                               m_messageIdCounter,
                                                               targetSite);
    m_controller->sendMessage(finishMessage);
    m_messageIdCounter++;
}

}
