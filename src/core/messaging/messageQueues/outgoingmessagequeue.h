#ifndef OUTGOINGMESSAGEQUEUE_H
#define OUTGOINGMESSAGEQUEUE_H

#include "messageQueue.h"

namespace KyoukoMind
{
class MessageController;

class OutgoingMessageQueue : public MessageQueue
{
public:
    OutgoingMessageQueue(const ClusterID clusterId,
                         MessageController *controller);

};

}

#endif // OUTGOINGMESSAGEQUEUE_H
