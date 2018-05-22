/**
 *  @file    incomingMessageBuffer.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef INCOMINGMESSAGEQUEUE_H
#define INCOMINGMESSAGEQUEUE_H

#include "messageBuffer.h"

namespace KyoukoMind
{
class Message;
class DataMessage;
class EdgeCluster;
class MessageController;

class IncomingMessageBuffer : public MessageBuffer
{
public:
    IncomingMessageBuffer(EdgeCluster *cluster,
                          MessageController *controller);

    bool addMessage(const uint8_t site, DataMessage *message);
    Message* getMessage(const uint8_t site);

    bool isReady() const;
    void resetFinishCounter();

private:
    std::mutex m_mutex;
    uint8_t m_finishCounter = 0;

    bool m_switchFlag = false;
    DataMessage* m_dataMessageBuffer1[17];
    DataMessage* m_dataMessageBuffer2[17];

    bool initMessageBuffer(EdgeCluster *cluster);
};

}

#endif // INCOMINGMESSAGEQUEUE_H
