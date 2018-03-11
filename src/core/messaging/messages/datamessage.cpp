#include "datamessage.h"

namespace KyoukoMind
{

/**
 * @brief DataMessage::DataMessage
 * @param clusterId
 * @param messageIdCounter
 * @param site
 */
DataMessage::DataMessage(const quint32 clusterId,
                         const quint32 messageIdCounter,
                         const quint8 site) :
    Message(clusterId, messageIdCounter, site)
{
    m_metaData.type = DATAMESSAGE;
    m_metaData.requiredReply = 1;
}

/**
 * @brief DataMessage::DataMessage
 */
DataMessage::DataMessage() : Message()
{}

/**
 * @brief DataMessage::convertFromByteArray
 * @param data
 * @return
 */
bool DataMessage::convertFromByteArray(const QByteArray &data)
{
    if(data.length() < sizeof(CommonMessageData) + 1) {
        return false;
    }
    const uint8_t* dataPointer = (uint8_t*)data.data();
    quint32 offset = convertCommonFromByteArray(dataPointer);
    m_numberOfEdges = dataPointer[offset];
    if(m_numberOfEdges > m_maxNumberOfEdges) {
        m_numberOfEdges = 0;
        return false;
    }
    memcpy((void*)(&m_edges),
           (void*)(dataPointer + offset + 1),
           sizeof(KyoChanEdge) * m_numberOfEdges);
    return true;
}

/**
 * @brief DataMessage::convertToByteArray
 * @return
 */
QByteArray DataMessage::convertToByteArray()
{
    // TODO: avoid too much data-copy
    QByteArray data = convertCommonToByteArray();
    data.append((char*)(&m_numberOfEdges), 1);
    data.append((char*)m_edges, sizeof(KyoChanEdge) * m_numberOfEdges);
    return data;
}

/**
 * @brief DataMessage::addEdge
 * @param newEdge
 * @return
 */
bool DataMessage::addEdge(const KyoChanEdge &newEdge)
{
    if(m_numberOfEdges < m_maxNumberOfEdges) {
        m_edges[m_numberOfEdges] = newEdge;
        m_numberOfEdges++;
        return true;
    }
    return false;
}

/**
 * @brief DataMessage::getNumberOfEdges
 * @return
 */
quint8 DataMessage::getNumberOfEdges() const
{
    return m_numberOfEdges;
}

/**
 * @brief DataMessage::getEdges
 * @return
 */
KyoChanEdge *DataMessage::getEdges() const
{
    return (KyoChanEdge*)(&m_edges);
}

}
