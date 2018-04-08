/**
 *  @file    messageBuffer.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "messageBuffer.h"
#include <core/messaging/messageController.h>
#include <core/messaging/messages/message.h>

namespace KyoukoMind
{

/**
 * @brief MessageBuffer::MessageBuffer
 * @param clusterId
 * @param controller
 */
MessageBuffer::MessageBuffer(const ClusterID clusterId,
                             MessageController *controller)
{
    m_clusterId = clusterId;
    m_controller = controller;
}

}
