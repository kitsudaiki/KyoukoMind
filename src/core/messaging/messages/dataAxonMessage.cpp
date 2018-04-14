/**
 *  @file    dataAxonMessage.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "dataAxonMessage.h"

namespace KyoukoMind
{

/**
 * @brief DataAxonMessage::DataAxonMessage
 * @param clusterId
 * @param messageIdCounter
 * @param site
 */
DataAxonMessage::DataAxonMessage(const ClusterID targetClusterId,
                                 const ClusterID sourceClusterId,
                                 const uint32_t messageIdCounter,
                                 const uint8_t targetSite) :
    Message(targetClusterId, sourceClusterId, messageIdCounter, targetSite)
{
    m_metaData.type = DATA_AXON_MESSAGE;
    m_metaData.requiredReply = 1;
}

/**
 * @brief DataAxonMessage::DataMessage
 */
DataAxonMessage::DataAxonMessage() : Message()
{}

/**
 * @brief DataAxonMessage::convertFromByteArray
 * @param data
 * @return
 */
bool DataAxonMessage::convertFromByteArray(uint8_t* data)
{
    if(data == nullptr) {
        return false;
    }
    uint32_t offset = convertCommonFromByteArray(data);
    m_numberOfAxonEdges = data[offset];
    if(m_numberOfAxonEdges > m_maxNumberOfAxonEdges) {
        m_numberOfAxonEdges = 0;
        return false;
    }
    memcpy((void*)(&m_edges),
           (void*)(data + offset + 1),
           sizeof(KyoChanAxonEdge) * m_numberOfAxonEdges);
    return true;
}

/**
 * @brief DataAxonMessage::convertToByteArray
 * @return
 */
uint8_t* DataAxonMessage::convertToByteArray()
{
    uint32_t size = (sizeof(KyoChanAxonEdge) * m_numberOfAxonEdges) + 1;
    uint8_t* data = convertCommonToByteArray(size);
    memcpy((void*)(data+sizeof(CommonMessageData)),
           (void*)(&m_numberOfAxonEdges),
           1);
    memcpy((void*)(data+sizeof(CommonMessageData)+1),
           m_edges,
           sizeof(KyoChanAxonEdge) * m_numberOfAxonEdges);
    return data;
}

/**
 * @brief DataAxonMessage::addAxonEdge
 * @param newEdge
 * @return
 */
bool DataAxonMessage::addAxonEdge(const KyoChanAxonEdge &newAxonEdge)
{
    if(m_numberOfAxonEdges < m_maxNumberOfAxonEdges) {
        m_edges[m_numberOfAxonEdges] = newAxonEdge;
        m_numberOfAxonEdges++;
        return true;
    }
    return false;
}

/**
 * @brief DataAxonMessage::getNumberOfAxonEdges
 * @return
 */
uint8_t DataAxonMessage::getNumberOfAxonEdges() const
{
    return m_numberOfAxonEdges;
}

/**
 * @brief DataAxonMessage::getAxonEdges
 * @return
 */
KyoChanAxonEdge *DataAxonMessage::getAxonEdges() const
{
    return (KyoChanAxonEdge*)(&m_edges);
}

}
