/**
 *  @file    dataMessage.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/messaging/messages/dataMessage.h>
#include <commonDataBuffer.h>

namespace KyoukoMind
{

/**
 * @brief DataMessage::DataMessage
 * @param clusterId
 * @param site
 */
DataMessage::DataMessage(const uint32_t targetClusterId,
                         const uint8_t targetSite,
                         const uint32_t sourceClusterId,
                         const uint64_t messageId) :
    Message(targetClusterId, targetSite, sourceClusterId)
{
    m_messageHeader.messageId = messageId;
    m_commonMessageInfo.type = DATA_MESSAGE;
    m_messageHeader.commonInfo = m_commonMessageInfo;

    m_buffer->addData(&m_messageHeader);
}

/**
 * @brief DataMessage::DataMessage
 */
DataMessage::DataMessage() :
    Message()
{
    m_commonMessageInfo.type = DATA_MESSAGE;
    m_messageHeader.commonInfo = m_commonMessageInfo;

    m_buffer->addData(&m_messageHeader);
}

/**
 * @brief DataMessage::~DataMessage
 */
DataMessage::~DataMessage()
{
    closeBuffer();
}

/**
 * @brief DataMessage::getHeaderFromBuffer
 * @return
 */
bool
DataMessage::getMetaDataFromBuffer()
{
    if(m_buffer == nullptr) {
        return false;
    }
    m_messageHeader = *((DataMessageHeader*)m_buffer->getBufferPointer());
    return true;
}

/**
 * @brief DataMessage::getMetaData
 * @return
 */
DataMessageHeader
DataMessage::getMetaData() const
{
    return m_messageHeader;
}

/**
 * @brief DataMessage::updateMessageHeader
 */
void
DataMessage::updateMessageHeader()
{
    memcpy(m_buffer->getBufferPointer(), (void*)(&m_messageHeader), sizeof(DataMessageHeader));
}

/**
 * @brief DataMessage::addData
 * @param data
 * @param size
 */
void
DataMessage::addData(void* data, const uint8_t size)
{
    m_buffer->addData(data, size);
    m_messageHeader.payloadSize += size;
}

/**
 * @brief DataMessage::getPayloadSize
 * @return
 */
uint32_t
DataMessage::getPayloadSize() const
{
    return m_messageHeader.payloadSize;
}

/**
 * @brief DataMessage::getPayload
 * @return
 */
uint8_t*
DataMessage::getPayload() const
{
    return m_buffer->getBufferPointer() + sizeof(DataMessageHeader);
}

/**
 * @brief DataMessage::setNumberOfActiveNodes
 * @param neighborInfo
 */
void
DataMessage::setNeighborInformation(const NeighborInformation neighborInfo)
{
    m_messageHeader.neighborInfos = neighborInfo;
}

}
