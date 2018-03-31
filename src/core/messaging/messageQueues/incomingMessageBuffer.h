#ifndef INCOMINGMESSAGEQUEUE_H
#define INCOMINGMESSAGEQUEUE_H

#include "messageBuffer.h"

namespace KyoukoMind
{
class MessageController;

class IncomingMessageBuffer : public MessageBuffer
{
public:
    IncomingMessageBuffer(const ClusterID clusterId,
                          MessageController *controller);

    bool addMessage(const uint8_t site, Message *message);
    std::vector<Message*>* getMessageQueue(const uint8_t site);

    bool isFinished() const;
    void resetFinishCounter();

private:
    std::mutex m_mutex;
    uint8_t m_finishCounter = 0;

    bool m_switchFlag = false;
    // TODO: make it right!!!!!!!!!!!!
    std::vector<Message*> m_messageQueue1[10];
    std::vector<Message*> m_messageQueue2[10];
};

}

#endif // INCOMINGMESSAGEQUEUE_H