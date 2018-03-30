#ifndef INCOMINGMESSAGEQUEUE_H
#define INCOMINGMESSAGEQUEUE_H

#include "messageQueue.h"

namespace KyoukoMind
{
class MessageController;

class IncomingMessageQueue : public MessageQueue
{
public:
    IncomingMessageQueue(const ClusterID clusterId,
                         MessageController *controller);

    bool addMessage(const uint8_t site, Message *message);
    bool isFinished() const;

private:
    uint8_t m_finishCounter = 0;
    std::vector<Message*> m_messages[10];
};

}

#endif // INCOMINGMESSAGEQUEUE_H
