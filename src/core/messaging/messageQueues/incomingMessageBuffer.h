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
class Cluster;
class MessageController;

class IncomingMessageBuffer : public MessageBuffer
{
public:
    IncomingMessageBuffer(Cluster *cluster,
                          MessageController *controller);

    bool addMessage(const uint8_t site, DataMessage *message);
    Message* getMessage(const uint8_t site);

    bool isReady() const;
    void resetFinishCounter();

private:
    std::mutex m_mutex;
    uint8_t m_finishCounter = 0;

    bool m_switchFlag = false;
    DataMessage* m_dataMessageBuffer1[16];
    DataMessage* m_dataMessageBuffer2[16];

    bool initMessageBuffer(Cluster *cluster);
};

}

#endif // INCOMINGMESSAGEQUEUE_H
