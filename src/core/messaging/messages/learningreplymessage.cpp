#include "learningreplymessage.h"

namespace KyoukoMind
{

/**
 * @brief LearningReplyMessage::LearningReplyMessage
 * @param clusterId
 * @param messageIdCounter
 * @param site
 */
LearningReplyMessage::LearningReplyMessage(const ClusterID targetClusterId,
                                           const uint32_t messageIdCounter,
                                           const uint8_t targetSite) :
    Message(targetClusterId, messageIdCounter, targetSite)
{
    m_metaData.type = LEARNINGREPLYMESSAGE;
    m_metaData.requiredReply = 1;
}

/**
 * @brief LearningReplyMessage::LearningReplyMessage
 */
LearningReplyMessage::LearningReplyMessage() : Message()
{}

/**
 * @brief LearningReplyMessage::convertFromByteArray
 * @param data
 * @return
 */
bool LearningReplyMessage::convertFromByteArray(uint8_t *data)
{
    if(data == nullptr) {
        return false;
    }
    uint32_t offset = convertCommonFromByteArray(data);
    m_numberOfNewEdgeReplys = data[offset];
    if(m_numberOfNewEdgeReplys > m_maxNumberOfNewEdgeReplys) {
        m_numberOfNewEdgeReplys = 0;
        return false;
    }
    memcpy((void*)(&m_newEdgeReplys),
           (void*)(data + offset + 1),
           sizeof(KyoChanNewEdgeReply) * m_numberOfNewEdgeReplys);
    return true;
}

/**
 * @brief LearningReplyMessage::convertToByteArray
 * @return
 */
uint8_t *LearningReplyMessage::convertToByteArray()
{
    uint32_t size = (sizeof(KyoChanNewEdgeReply) * m_numberOfNewEdgeReplys) + 1;
    uint8_t* data = convertCommonToByteArray(size);
    memcpy((void*)(data+sizeof(CommonMessageData)),
           (void*)(&m_numberOfNewEdgeReplys),
           1);
    memcpy((void*)(data+sizeof(CommonMessageData)+1),
           m_newEdgeReplys,
           sizeof(KyoChanNewEdgeReply) * m_numberOfNewEdgeReplys);
    return data;
}

/**
 * @brief LearningReplyMessage::addNewEdgeReply
 * @param newEdgeReply
 * @return
 */
bool LearningReplyMessage::addNewEdgeReply(const KyoChanNewEdgeReply &newEdgeReply)
{
    if(m_numberOfNewEdgeReplys < m_maxNumberOfNewEdgeReplys) {
        m_newEdgeReplys[m_numberOfNewEdgeReplys] = newEdgeReply;
        m_numberOfNewEdgeReplys++;
        return true;
    }
    return false;
}

/**
 * @brief LearningReplyMessage::getNumberOfEdgeReplys
 * @return
 */
uint8_t LearningReplyMessage::getNumberOfEdgeReplys() const
{
    return m_numberOfNewEdgeReplys;
}

/**
 * @brief LearningReplyMessage::getNewEdgeReplys
 * @return
 */
KyoChanNewEdgeReply *LearningReplyMessage::getNewEdgeReplys() const
{
    return (KyoChanNewEdgeReply*)(&m_newEdgeReplys);
}

}
