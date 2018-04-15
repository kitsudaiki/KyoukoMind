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
class Cluster;

class MessageController;
class DataMessage;
class RelyMessage;

class OutgoingMessageBuffer : public MessageBuffer
{
public:
    OutgoingMessageBuffer(Cluster *cluster,
                          MessageController *controller);

    bool addEdge(const uint8_t sourceSite,
                 const KyoChanMessageEdge *edge);
    bool addAxonEdge(const uint8_t sourceSite,
                     const KyoChanAxonEdge *newAxonEdge);
    bool addLearingEdge(const uint8_t sourceSite,
                        const KyoChanNewEdge *newEdge);
    bool addLearningReplyMessage(const uint8_t sourceSite,
                                 const KyoChanNewEdgeReply *newEdgeReply);

    void sendReplyMessage(const uint8_t sourceSite);

    void sendFinishCycle(const uint8_t sourceSite);

private:
    DataMessage* m_dataMessageBuffer[16];
    uint32_t m_messageIdCounter = 0;
};

}

#endif // OUTGOINGMESSAGEQUEUE_H
