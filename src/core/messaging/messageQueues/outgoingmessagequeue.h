#ifndef OUTGOINGMESSAGEQUEUE_H
#define OUTGOINGMESSAGEQUEUE_H

#include "messageQueue.h"

namespace KyoukoMind
{
class MessageController;
class DataMessage;
class LearningMessage;

class OutgoingMessageQueue : public MessageQueue
{
public:
    OutgoingMessageQueue(const ClusterID clusterId,
                         MessageController *controller);

    bool addEdge(const ClusterID targetClusterId,
                 const uint8_t targetSite,
                 const KyoChanEdge newEdge);
    bool addLearingEdge(const ClusterID targetClusterId,
                        const uint8_t targetSite,
                        const KyoChanNewEdge newEdge);

    void sendReplyMessage(const ClusterID targetClusterId,
                          const uint8_t targetSite);
    void sendLearningReplyMessage(const ClusterID targetClusterId,
                                  const uint8_t targetSite);

    void sendFinishCycle(const ClusterID targetClusterId,
                         const uint8_t targetSite);

private:
    DataMessage* m_dataMessageBuffer[10];
    LearningMessage* m_learingMessageBuffer[10];
    uint32_t m_messageIdCounter = 0;
};

}

#endif // OUTGOINGMESSAGEQUEUE_H
