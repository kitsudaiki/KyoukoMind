#include "learningreplymessage.h"

namespace KyoukoMind
{

/**
 * @brief LearningReplyMessage::LearningReplyMessage
 * @param clusterId
 * @param messageIdCounter
 * @param site
 */
LearningReplyMessage::LearningReplyMessage(const quint32 clusterId,
                                           const quint32 messageIdCounter,
                                           const quint8 site) :
    Message(clusterId, messageIdCounter, site)
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
bool LearningReplyMessage::convertFromByteArray(const QByteArray &data)
{
    if(data.length() < sizeof(CommonMessageData) + 1) {
        return false;
    }
    const uint8_t* dataPointer = (uint8_t*)data.data();
    quint32 offset = convertCommonFromByteArray(dataPointer);
    m_numberOfNewEdgeReplys = dataPointer[offset];
    if(m_numberOfNewEdgeReplys > m_maxNumberOfNewEdgeReplys) {
        m_numberOfNewEdgeReplys = 0;
        return false;
    }
    memcpy((void*)(&m_newEdgeReplys),
           (void*)(dataPointer + offset + 1),
           sizeof(KyoChanNewEdgeReply) * m_numberOfNewEdgeReplys);
    return true;
}

/**
 * @brief LearningReplyMessage::convertToByteArray
 * @return
 */
QByteArray LearningReplyMessage::convertToByteArray()
{
    // TODO: avoid too much data-copy
    QByteArray data = convertCommonToByteArray();
    data.append((char*)(&m_numberOfNewEdgeReplys), 1);
    data.append((char*)m_newEdgeReplys, sizeof(KyoChanNewEdgeReply) * m_numberOfNewEdgeReplys);
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
quint8 LearningReplyMessage::getNumberOfEdgeReplys() const
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
