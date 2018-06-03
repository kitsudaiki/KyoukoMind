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

    bool addStatusEdge(const uint8_t sourceSide,
                       const KyoChanStatusEdgeContainer *edge);
    bool addInternalEdge(const uint8_t sourceSide,
                         const KyoChanInternalEdgeContainer *edge);
    bool addDirectEdge(const uint8_t sourceSide,
                       const KyoChanDirectEdgeContainer *edge);
    bool addForwardEdge(const uint8_t sourceSide,
                        const KyoChanForwardEdgeContainer *edge);
    bool addAxonEdge(const uint8_t sourceSide,
                     const KyoChanAxonEdgeContainer *newAxonEdge);
    bool addLearingEdge(const uint8_t sourceSide,
                        const KyoChanLearingEdgeContainer *newEdge);
    bool addLearningReplyMessage(const uint8_t sourceSide,
                                 const KyoChanLearningEdgeReplyContainer *newEdgeReply);
    bool addDeleteMessage(const uint8_t sourceSide,
                          const KyoChanDeleteContainer *edge);

    void sendReplyMessage(const uint8_t sourceSide);

    void finishCycle(const uint8_t sourceSide, const uint16_t numberOfActiveNodes);

    bool updateBufferInit();
private:
    DataMessage* m_dataMessageBuffer[17];
    uint32_t m_messageIdCounter = 0;
};

}

#endif // OUTGOINGMESSAGEQUEUE_H
