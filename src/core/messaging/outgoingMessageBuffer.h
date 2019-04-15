/**
 *  @file    outgingMessageBuffer.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef OUTGOINGMESSAGEQUEUE_H
#define OUTGOINGMESSAGEQUEUE_H

#include <common.h>
#include <core/structs/messageContainer.h>
#include <core/messaging/messages/dataMessage.h>

namespace KyoukoMind
{

class IncomingMessageBuffer;

class OutgoingMessageBuffer
{
public:
    OutgoingMessageBuffer(const uint32_t targetClusterId,
                          const uint8_t targetSite,
                          const uint32_t sourceClusterId);
    OutgoingMessageBuffer();


    template <typename T>
    bool addData(T *data)
    {
        if(data != nullptr) {
            m_currentMessage->addData((void*)data, sizeof(T));
            return true;
        }
        return false;
    }

    bool finishCycle(const uint32_t targetClusterId,
                     const uint8_t targetSite,
                     const uint32_t sourceClusterId,
                     const NeighborInformation neighborInfo);
    void setIncomingBuffer(IncomingMessageBuffer* incomingBuffer);

private:
    DataMessage* m_currentMessage = nullptr;

    uint32_t m_messageIdCounter = 0;
    uint32_t m_clusterId = 0;
    uint8_t m_side = 0;

    uint32_t m_targetClusterId = 0;
    uint8_t m_targetSite = 0;
    uint32_t m_sourceClusterId = 0;

    IncomingMessageBuffer *m_incomingBuffer = nullptr;

    uint64_t getMessageId();
};

}

#endif // OUTGOINGMESSAGEQUEUE_H
