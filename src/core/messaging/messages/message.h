/**
 *  @file    message.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <common.h>
#include <core/structs/messageHeader.h>

namespace Kitsune
{
class CommonDataBuffer;
}

namespace KyoukoMind
{

class Message
{
public:
    Message(const uint32_t targetClusterId,
            const uint8_t targetSite,
            const uint32_t sourceClusterId);
    Message();
    virtual ~Message();

    void setCommonInfo(const uint32_t targetClusterId,
                       const uint8_t targetSite,
                       const uint32_t sourceClusterId);

    bool closeBuffer();
    uint8_t getType() const;

    uint32_t getDataSize() const;
    uint8_t* getData() const;
    uint64_t getTotalDataSize() const;

    virtual bool getMetaDataFromBuffer() = 0;
    virtual uint32_t getPayloadSize() const = 0;
    virtual uint8_t* getPayload() const = 0;

protected:
    Kitsune::CommonDataBuffer* m_buffer = nullptr;
    CommonMessageInformation m_commonMessageInfo;

private:
    void initBuffer();
};

}

#endif // MESSAGE_H
