#include "replymessage.h"

namespace KyoukoMind
{

/**
 * @brief ReplyMessage::ReplyMessage
 * @param messageId
 * @param site
 */
ReplyMessage::ReplyMessage(const ClusterID targetClusterId,
                           const uint32_t messageId,
                           const uint8_t targetSite) :
    Message(targetClusterId, messageId, targetSite)
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
bool ReplyMessage::convertFromByteArray(uint8_t *data)
{
    if(data == nullptr) {
        return false;
    }
    convertCommonFromByteArray(data);
    return true;
}

/**
 * @brief ReplyMessage::convertToByteArray
 * @return
 */
uint8_t* ReplyMessage::convertToByteArray()
{
    return convertCommonToByteArray(0);
}

}
