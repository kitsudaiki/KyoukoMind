#include "message.h"

namespace KyoukoMind
{

/**
 * @brief Message::Message
 * @param clusterId
 * @param messageIdCounter
 * @param site
 */
Message::Message(const ClusterID targetClusterId,
                 const ClusterID sourceClusterId,
                 const uint32_t messageIdCounter,
                 const uint8_t site)
{
    m_metaData.targetClusterId = targetClusterId;
    m_metaData.messageId = messageIdCounter;
    m_metaData.messageId = m_metaData.messageId << 32;
    m_metaData.messageId += sourceClusterId;
    m_metaData.site = site;
}

/**
 * @brief Message::Message
 * @param messageId
 * @param site
 */
Message::Message(const ClusterID targetClusterId,
                 const uint64_t messageId,
                 const uint8_t site)
{
    m_metaData.targetClusterId = targetClusterId;
    m_metaData.messageId = messageId;
    m_metaData.site = site;
}

/**
 * @brief Message::Message
 */
Message::Message()
{}

/**
 * @brief Message::metaData
 * @return
 */
CommonMessageData Message::getMetaData() const
{
    return m_metaData;
}

/**
 * @brief Message::getType
 * @return
 */
uint8_t Message::getType() const
{
    return m_metaData.type;
}

/**
 * @brief Message::convertCommonToByteArray
 * @param size
 * @return
 */
uint8_t* Message::convertCommonToByteArray(const uint32_t size)
{
    uint8_t* data = new uint8_t[sizeof(CommonMessageData) + size];
    memcpy((void*)(data), (void*)(&m_metaData), sizeof(CommonMessageData));
    return data;
}

/**
 * @brief Message::convertCommonFromByteArray
 * @return
 */
uint32_t Message::convertCommonFromByteArray(const uint8_t* data)
{
    memcpy((void*)(&m_metaData), (void*)data, sizeof(CommonMessageData));
    return sizeof(CommonMessageData);
}

}
