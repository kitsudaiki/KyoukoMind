/**
 *  @file    replyMessage.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "replyMessage.h"
#include <files/dataBuffer.h>

namespace KyoukoMind
{

/**
 * @brief ReplyMessage::ReplyMessage
 * @param messageId
 * @param site
 */
ReplyMessage::ReplyMessage(const ClusterID targetClusterId,
                           const uint8_t targetSite) :
    Message(targetClusterId, targetSite)
{
    m_metaData.type = REPLY_MESSAGE;
    m_metaData.requiredReply = 0;

    memcpy(m_buffer->getBufferPointer(), (void*)(&m_metaData), sizeof(CommonMessageData));
    m_currentBufferPos = sizeof(CommonMessageData);
}

/**
 * @brief ReplyMessage::ReplyMessage
 * @param data
 */
ReplyMessage::ReplyMessage(void *data, uint32_t size) : Message(data, size)
{}

}
