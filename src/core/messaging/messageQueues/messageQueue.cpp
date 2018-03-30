#include "messageQueue.h"
#include <core/messaging/messagecontroller.h>
#include <core/messaging/messages/message.h>

namespace KyoukoMind
{

MessageQueue::MessageQueue(const ClusterID clusterId,
                           MessageController *controller)
{
    m_clusterId = clusterId;
    m_controller = controller;
}

}
