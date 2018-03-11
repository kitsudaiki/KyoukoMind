#include "learningreplymessage.h"

/**
 * @brief LearningReplyMessage::LearningReplyMessage
 * @param messageId
 * @param site
 */
LearningReplyMessage::LearningReplyMessage(const quint32 messageId,
                                           const quint8 site) :
    Message(messageId, site)
{
    m_metaData.type = LEARNINGREPLYMESSAGE;
    m_metaData.requiredReploy = 0;
}

/**
 * @brief LearningReplyMessage::LearningReplyMessage
 */
LearningReplyMessage::LearningReplyMessage() : Message()
{}

/**
 * @brief LearningReplyMessage::convertFromByteArray
 * @param data
 * @return
 */
bool LearningReplyMessage::convertFromByteArray(const QByteArray &data)
{
    if(data.length() < sizeof(CommonMessageData)) {
        return false;
    }
    convertCommonFromByteArray((uint8_t*)data.data());
    return true;
}

/**
 * @brief LearningReplyMessage::convertToByteArray
 * @return
 */
QByteArray LearningReplyMessage::convertToByteArray()
{
    return convertCommonToByteArray();
}
