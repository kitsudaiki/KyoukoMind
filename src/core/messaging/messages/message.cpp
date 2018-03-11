#include "message.h"

namespace KyoukoMind
{

/**
 * @brief Message::Message
 * @param clusterId
 * @param messageIdCounter
 * @param site
 */
Message::Message(const quint32 clusterId,
                 const quint32 messageIdCounter,
                 const quint8 site)
{
    m_metaData.messageId = messageIdCounter;
    m_metaData.messageId = m_metaData.messageId << 32;
    m_metaData.messageId += clusterId;
    m_metaData.site = site;
}

/**
 * @brief Message::Message
 * @param messageId
 * @param site
 */
Message::Message(const quint64 messageId,
                 const quint8 site)
{
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
 * @brief Message::convertCommonToByteArray
 * @return
 */
QByteArray Message::convertCommonToByteArray()
{
    QByteArray data;
    data.append((char*)(&m_metaData), sizeof(CommonMessageData));
    return data;
}

/**
 * @brief Message::convertCommonFromByteArray
 * @return
 */
quint32 Message::convertCommonFromByteArray(const uint8_t* data)
{
    memcpy((void*)(&m_metaData), (void*)data, sizeof(CommonMessageData));
    return sizeof(CommonMessageData);
}

}
