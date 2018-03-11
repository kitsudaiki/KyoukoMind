#include "learningmessage.h"

namespace KyoukoMind
{

/**
 * @brief LearningMessage::LearningMessage
 * @param clusterId
 * @param messageIdCounter
 * @param site
 */
LearningMessage::LearningMessage(const quint32 clusterId,
                                 const quint32 messageIdCounter,
                                 const quint8 site) :
    Message(clusterId, messageIdCounter, site)
{
    m_metaData.type = LEARNINGMESSAGE;
    m_metaData.requiredReply = 1;
}

/**
 * @brief LearningMessage::LearningMessage
 */
LearningMessage::LearningMessage() : Message()
{}

/**
 * @brief LearningMessage::convertFromByteArray
 * @param data
 * @return
 */
bool LearningMessage::convertFromByteArray(const QByteArray &data)
{
    if(data.length() < sizeof(CommonMessageData) + 1) {
        return false;
    }
    const uint8_t* dataPointer = (uint8_t*)data.data();
    quint32 offset = convertCommonFromByteArray(dataPointer);
    m_numberOfNewEdges = dataPointer[offset];
    if(m_numberOfNewEdges > m_maxNumberOfNewEdges) {
        m_numberOfNewEdges = 0;
        return false;
    }
    memcpy((void*)(&m_newEdges),
           (void*)(dataPointer + offset + 1),
           sizeof(KyoChanNewEdge) * m_numberOfNewEdges);
    return true;
}

/**
 * @brief LearningMessage::convertToByteArray
 * @return
 */
QByteArray LearningMessage::convertToByteArray()
{
    // TODO: avoid too much data-copy
    QByteArray data = convertCommonToByteArray();
    data.append((char*)(&m_numberOfNewEdges), 1);
    data.append((char*)m_newEdges, sizeof(KyoChanNewEdge) * m_numberOfNewEdges);
    return data;
}

/**
 * @brief LearningMessage::addNewEdge
 * @param newEdge
 * @return
 */
bool LearningMessage::addNewEdge(const KyoChanNewEdge &newEdge)
{
    if(m_numberOfNewEdges < m_maxNumberOfNewEdges) {
        m_newEdges[m_numberOfNewEdges] = newEdge;
        m_numberOfNewEdges++;
        return true;
    }
    return false;
}

/**
 * @brief LearningMessage::getNumberOfEdges
 * @return
 */
quint8 LearningMessage::getNumberOfEdges() const
{
    return m_numberOfNewEdges;
}

/**
 * @brief LearningMessage::getNewEdges
 * @return
 */
KyoChanNewEdge *LearningMessage::getNewEdges() const
{
    return (KyoChanNewEdge*)(&m_newEdges);
}

}
