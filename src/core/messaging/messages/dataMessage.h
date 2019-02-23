/**
 *  @file    dataMessage.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef DATAMESSAGE_H
#define DATAMESSAGE_H

#include "message.h"
#include <common.h>

namespace KyoukoMind
{

class DataMessage : public Message
{
public:
    DataMessage(const uint32_t targetClusterId,
                const uint8_t targetSite,
                const uint32_t sourceClusterId,
                const uint64_t messageId = 0);
    DataMessage();
    ~DataMessage();

    bool getMetaDataFromBuffer();
    DataMessageHeader getMetaData() const;

    void addData(void *data, const uint8_t size);

    uint32_t getPayloadSize() const;
    uint8_t *getPayload() const;

    void setNeighborInformation(const NeighborInformation neighborInfo);
    void updateMessageHeader();

private:
    void copyToBuffer(void* data, const uint8_t size);
    void checkBuffer(const uint8_t size);

    DataMessageHeader m_messageHeader;
};

}

#endif // DATAMESSAGE_H
