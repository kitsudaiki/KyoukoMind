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
 * @param cluster
 * @param controller
 */
MessageBuffer::MessageBuffer(EdgeCluster* cluster,
                             MessageController *controller)
{
    m_cluster = cluster;
    m_controller = controller;
}

}
