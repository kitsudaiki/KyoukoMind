#include "outgoingmessagequeue.h"

#include <core/messaging/messagecontroller.h>

namespace KyoukoMind
{

OutgoingMessageQueue::OutgoingMessageQueue(const ClusterID clusterId,
                                           MessageController* controller):
    MessageQueue(clusterId, controller)
{

}

}
