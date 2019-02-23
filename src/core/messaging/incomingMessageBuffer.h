/**
 *  @file    incomingMessageBuffer.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef INCOMINGMESSAGEQUEUE_H
#define INCOMINGMESSAGEQUEUE_H

#include <common.h>

#define INCOM_BUFFER_SIZE 10

namespace KyoukoMind
{
class Message;
class DataMessage;
class Cluster;
class MessageController;

class IncomingMessageBuffer
{

public:
    IncomingMessageBuffer();
    ~IncomingMessageBuffer();

    bool addMessage(DataMessage *message);
    DataMessage *getMessage();

    bool finish();
    bool isReady();

private:
    std::mutex m_mutex;

    DataMessage* m_waitingMessages[INCOM_BUFFER_SIZE];
    DataMessage* m_currentProcessingMessage = nullptr;

    uint8_t m_newestBufferPos = 0;
    uint8_t m_oldestBufferPos = 0;

    bool clearCurrentMessage();
};

}

#endif // INCOMINGMESSAGEQUEUE_H
