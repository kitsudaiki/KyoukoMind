/**
 *  @file    learningMessage.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "learningMessage.h"

namespace KyoukoMind
{

/**
 * @brief LearningMessage::LearningMessage
 * @param clusterId
 * @param messageIdCounter
 * @param site
 */
LearningMessage::LearningMessage(const ClusterID targetClusterId,
                                 const uint32_t sourceClusterId,
                                 const uint32_t messageIdCounter,
                                 const uint8_t targetSite) :
    Message(targetClusterId, sourceClusterId, messageIdCounter, targetSite)
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
bool LearningMessage::convertFromByteArray(uint8_t *data)
{
    if(data == nullptr) {
        return false;
    }
    uint32_t offset = convertCommonFromByteArray(data);
    m_numberOfNewEdges = data[offset];
    if(m_numberOfNewEdges > m_maxNumberOfNewEdges) {
        m_numberOfNewEdges = 0;
        return false;
    }
    memcpy((void*)(&m_newEdges),
           (void*)(data + offset + 1),
           sizeof(KyoChanNewEdge) * m_numberOfNewEdges);
    return true;
}

/**
 * @brief LearningMessage::convertToByteArray
 * @return
 */
uint8_t *LearningMessage::convertToByteArray()
{
    uint32_t size = (sizeof(KyoChanNewEdge) * m_numberOfNewEdges) + 1;
    uint8_t* data = convertCommonToByteArray(size);
    memcpy((void*)(data+sizeof(CommonMessageData)),
           (void*)(&m_numberOfNewEdges),
           1);
    memcpy((void*)(data+sizeof(CommonMessageData)+1),
           m_newEdges,
           sizeof(KyoChanNewEdge) * m_numberOfNewEdges);
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
uint8_t LearningMessage::getNumberOfNewEdges() const
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
