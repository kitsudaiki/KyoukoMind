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
class EdgeCluster;

class MessageController;
class DataMessage;
class RelyMessage;

class OutgoingMessageBuffer : public MessageBuffer
{
public:
    OutgoingMessageBuffer(EdgeCluster *cluster,
                          MessageController *controller);

    bool addStatusEdge(const uint8_t sourceSite,
                       const KyoChanStatusEdgeContainer *edge);
    bool addInternalEdge(const uint8_t sourceSite,
                         const KyoChanInternalEdgeContainer *edge);
    bool addDirectEdge(const uint8_t sourceSite,
                       const KyoChanDirectEdgeContainer *edge);
    bool addForwardEdge(const uint8_t sourceSite,
                        const KyoChanForwardEdgeContainer *edge);
    bool addAxonEdge(const uint8_t sourceSite,
                     const KyoChanAxonEdgeContainer *newAxonEdge);
    bool addLearingEdge(const uint8_t sourceSite,
                        const KyoChanLearingEdgeContainer *newEdge);
    bool addLearningReplyMessage(const uint8_t sourceSite,
                                 const KyoChanLearningEdgeReplyContainer *newEdgeReply);

    void sendReplyMessage(const uint8_t sourceSite);

    void finishCycle(const uint8_t sourceSite, const uint16_t numberOfActiveNodes);

    bool updateBufferInit();
private:
    DataMessage* m_dataMessageBuffer[17];
    uint32_t m_messageIdCounter = 0;
};

}

#endif // OUTGOINGMESSAGEQUEUE_H
