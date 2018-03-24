#include "datamessage.h"

namespace KyoukoMind
{

/**
 * @brief DataMessage::DataMessage
 * @param clusterId
 * @param messageIdCounter
 * @param site
 */
DataMessage::DataMessage(const uint32_t clusterId,
                         const uint32_t messageIdCounter,
                         const uint8_t site) :
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
bool DataMessage::convertFromByteArray(uint8_t* data)
{
    if(data == nullptr) {
        return false;
    }
    uint32_t offset = convertCommonFromByteArray(data);
    m_numberOfEdges = data[offset];
    if(m_numberOfEdges > m_maxNumberOfEdges) {
        m_numberOfEdges = 0;
        return false;
    }
    memcpy((void*)(&m_edges),
           (void*)(data + offset + 1),
           sizeof(KyoChanEdge) * m_numberOfEdges);
    return true;
}

/**
 * @brief DataMessage::convertToByteArray
 * @return
 */
uint8_t* DataMessage::convertToByteArray()
{
    uint32_t size = (sizeof(KyoChanEdge) * m_numberOfEdges) + 1;
    uint8_t* data = convertCommonToByteArray(size);
    memcpy((void*)(data+sizeof(CommonMessageData)),
           (void*)(&m_numberOfEdges),
           1);
    memcpy((void*)(data+sizeof(CommonMessageData)+1),
           m_edges,
           sizeof(KyoChanEdge) * m_numberOfEdges);
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
uint8_t DataMessage::getNumberOfEdges() const
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
