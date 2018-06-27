/**
 *  @file    outgoingMessageBuffer.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "outgoingMessageBuffer.h"

#include <core/clustering/cluster/edgeCluster.h>
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
    for(uint32_t side = 0; side < 17; side++)
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
    for(uint32_t side = 0; side < 17; side++)
    {
        if(m_dataMessageBuffer[side] != nullptr) {
            delete m_dataMessageBuffer[side];
        }

        uint64_t id = m_messageIdCounter;
        id = (id << 32) + m_cluster->getClusterId();
        m_dataMessageBuffer[side] = new DataMessage(m_cluster->getNeighborId(side),
                                                    m_cluster->getClusterId(),
                                                    16 - side,
                                                    id);
    }
}

/**
 * @brief OutgoingMessageBuffer::addStatusEdge
 * @param sourceSide
 * @param edge
 * @return
 */
bool OutgoingMessageBuffer::addStatusEdge(const uint8_t sourceSide,
                                          const KyoChanStatusEdgeContainer *edge)
{
    if(sourceSide < 17) {
        m_dataMessageBuffer[sourceSide]->addStatusEdge(edge);
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::addEdge
 * @param sourceSide
 * @param edge
 * @return
 */
bool OutgoingMessageBuffer::addPendingEdge(const uint8_t sourceSide,
                                           const KyoChanPendingEdgeContainer *edge)
{
    if(sourceSide < 17) {
        m_dataMessageBuffer[sourceSide]->addPendingEdge(edge);
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::addDirectEdge
 * @param targetSite
 * @param newEdge
 * @return
 */
bool OutgoingMessageBuffer::addDirectEdge(const uint8_t sourceSide,
                                          const KyoChanDirectEdgeContainer *edge)
{
    if(sourceSide < 17) {
        m_dataMessageBuffer[sourceSide]->addDirectEdge(edge);
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::addForwardEdge
 * @param targetSite
 * @param newEdge
 * @return
 */
bool OutgoingMessageBuffer::addForwardEdge(const uint8_t sourceSide,
                                           const KyoChanForwardEdgeContainer *edge)
{
    if(sourceSide < 17) {
        m_dataMessageBuffer[sourceSide]->addForwardEdge(edge);
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
bool OutgoingMessageBuffer::addAxonEdge(const uint8_t sourceSide,
                                        const KyoChanAxonEdgeContainer *newAxonEdge)
{
    if(sourceSide < 17) {
        m_dataMessageBuffer[sourceSide]->addAxonEdge(newAxonEdge);
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
bool OutgoingMessageBuffer::addLearingEdge(const uint8_t sourceSide,
                                           const KyoChanLearingEdgeContainer *newEdge)
{
    if(sourceSide <= 17) {
        m_dataMessageBuffer[sourceSide]->addLearningEdge(newEdge);
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::addLearningReplyMessage
 * @param targetClusterId
 * @param newEdgeReply
 */
bool OutgoingMessageBuffer::addLearningReplyMessage(const uint8_t sourceSide,
                                                    const KyoChanLearningEdgeReplyContainer *newEdgeReply)
{
    if(sourceSide <= 17) {
        m_dataMessageBuffer[sourceSide]->addLearningReplyEdgeReply(newEdgeReply);
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::addDeleteMessage
 * @param sourceSide
 * @param edge
 * @return
 */
bool OutgoingMessageBuffer::addDeleteMessage(const uint8_t sourceSide,
                                             const KyoChanDeleteContainer *edge)
{
    if(sourceSide <= 17) {
        m_dataMessageBuffer[sourceSide]->addDeleteEdge(edge);
        return true;
    }
    return false;
}

/**
 * @brief OutgoingMessageBuffer::sendReplyMessage
 * @param sourceSide
 */
void OutgoingMessageBuffer::sendReplyMessage(const uint8_t sourceSide)
{
    ReplyMessage* replyMessage = new ReplyMessage(m_cluster->getNeighborId(sourceSide),
                                                  16 - sourceSide);
    m_controller->sendMessage(replyMessage);
    m_messageIdCounter++;
}

/**
 * @brief OutgoingMessageBuffer::finishCycle
 * @param sourceSide
 * @param numberOfActiveNodes
 */
void OutgoingMessageBuffer::finishCycle(const uint8_t sourceSide,
                                        const uint16_t numberOfActiveNodes)
{
    m_dataMessageBuffer[sourceSide]->setNumberOfActiveNodes(numberOfActiveNodes);
    m_controller->sendMessage(m_dataMessageBuffer[sourceSide]);

    m_messageIdCounter++;
    uint64_t cluster_id = m_cluster->getClusterId();
    uint32_t id = m_messageIdCounter;
    id = (cluster_id << 32) + id;

    m_dataMessageBuffer[sourceSide] = new DataMessage(m_cluster->getNeighborId(sourceSide),
                                                      m_cluster->getClusterId(),
                                                      16 - sourceSide,
                                                      id);
}

}
