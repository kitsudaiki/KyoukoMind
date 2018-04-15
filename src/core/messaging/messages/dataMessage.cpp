/**
 *  @file    dataMessage.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "dataMessage.h"
#include <files/dataBuffer.h>

namespace KyoukoMind
{

/**
 * @brief DataMessage::DataMessage
 * @param clusterId
 * @param site
 */
DataMessage::DataMessage(const ClusterID targetClusterId,
                         const ClusterID sourceClusterId,
                         const uint8_t targetSite) :
    Message(targetClusterId, sourceClusterId, targetSite)
{
    m_metaData.type = DATA_MESSAGE;
    m_metaData.requiredReply = 1;

    memcpy(m_buffer->getBufferPointer(), (void*)(&m_metaData), sizeof(CommonMessageData));
    m_currentBufferPos = sizeof(CommonMessageData);
}

/**
 * @brief DataMessage::DataMessage
 * @param data
 */
DataMessage::DataMessage(void *data, uint32_t size) : Message(data, size)
{}

/**
 * @brief DataMessage::addEdge
 * @param newEdge
 */
void DataMessage::addEdge(const KyoChanMessageEdge *newEdge)
{
    checkBuffer();
    copyToBuffer((void*)newEdge);
}

/**
 * @brief DataMessage::addAxonEdge
 * @param newAxonEdge
 */
void DataMessage::addAxonEdge(const KyoChanAxonEdge* newAxonEdge)
{
    checkBuffer();
    copyToBuffer((void*)newAxonEdge);
}

/**
 * @brief DataMessage::addNewEdge
 * @param newEdge
 */
void DataMessage::addNewEdge(const KyoChanNewEdge *newEdge)
{
    checkBuffer();
    copyToBuffer((void*)newEdge);
}

/**
 * @brief DataMessage::addNewEdgeReply
 * @param newEdgeReply
 */
void DataMessage::addNewEdgeReply(const KyoChanNewEdgeReply *newEdgeReply)
{
    checkBuffer();
    copyToBuffer((void*)newEdgeReply);
}

/**
 * @brief DataMessage::getNumberOfEdges
 * @return
 */
uint8_t DataMessage::getNumberOfEntries() const
{
    return (m_currentBufferPos - sizeof(CommonMessageData)) / 20;
}

/**
 * @brief DataMessage::checkBuffer
 */
void DataMessage::checkBuffer()
{
    if(m_currentBufferPos + 20 > m_currentBufferSize) {
        m_buffer->allocateBlocks(1);
        m_currentBufferSize += m_buffer->getBlockSize() * m_buffer->getNumberOfBlocks();
    }
}

/**
 * @brief DataMessage::copyToBuffer
 * @param data
 */
void DataMessage::copyToBuffer(void *data)
{
    memcpy(m_buffer->getBufferPointer() + m_currentBufferPos,
           data,
           20);
    m_currentBufferPos += 20;
}

}
