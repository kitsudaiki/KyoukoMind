/**
 *  @file    outgingMessageBuffer.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef OUTGOINGMESSAGEQUEUE_H
#define OUTGOINGMESSAGEQUEUE_H

#include "messageBuffer.h"

namespace KyoukoMind
{
class MessageController;
class DataMessage;
class DataAxonMessage;
class LearningMessage;
class LearningReplyMessage;

class OutgoingMessageBuffer : public MessageBuffer
{
public:
    OutgoingMessageBuffer(const ClusterID clusterId,
                          MessageController *controller);

    bool addEdge(const ClusterID targetClusterId,
                 const uint8_t targetSite,
                 const KyoChanEdge newEdge);
    bool addAxonEdge(const ClusterID targetClusterId,
                     const uint8_t targetSite,
                     const KyoChanAxonEdge newAxonEdge);
    bool addLearingEdge(const ClusterID targetClusterId,
                        const uint8_t targetSite,
                        const KyoChanNewEdge newEdge);
    bool addLearningReplyMessage(const ClusterID targetClusterId,
                                 const uint8_t targetSite,
                                 const KyoChanNewEdgeReply newEdgeReply);

    void sendReplyMessage(const ClusterID targetClusterId,
                          const uint8_t targetSite);

    void sendFinishCycle(const ClusterID targetClusterId,
                         const uint8_t targetSite);

private:
    DataMessage* m_dataMessageBuffer[16];
    DataAxonMessage* m_dataAxonMessageBuffer[16];
    LearningMessage* m_learingMessageBuffer[16];
    LearningReplyMessage* m_learingReplyMessageBuffer[16];
    uint32_t m_messageIdCounter = 0;
};

}

#endif // OUTGOINGMESSAGEQUEUE_H
