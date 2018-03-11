#include "replymessage.h"

namespace KyoukoMind
{

/**
 * @brief ReplyMessage::ReplyMessage
 * @param messageId
 * @param site
 */
ReplyMessage::ReplyMessage(const quint32 messageId,
                           const quint8 site) :
    Message(messageId, site)
{
    m_metaData.type = REPLYMESSAGE;
    m_metaData.requiredReply = 0;
}

/**
 * @brief ReplyMessage::ReplyMessage
 */
ReplyMessage::ReplyMessage() : Message()
{}

/**
 * @brief ReplyMessage::convertFromByteArray
 * @param data
 * @return
 */
bool ReplyMessage::convertFromByteArray(const QByteArray &data)
{
    if(data.length() < sizeof(CommonMessageData)) {
        return false;
    }
    convertCommonFromByteArray((uint8_t*)data.data());
    return true;
}

/**
 * @brief ReplyMessage::convertToByteArray
 * @return
 */
QByteArray ReplyMessage::convertToByteArray()
{
    return convertCommonToByteArray();
}

}
