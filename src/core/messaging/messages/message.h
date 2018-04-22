/**
 *  @file    message.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <common.h>
#include <core/structs/clusterMeta.h>
#include <core/structs/kyochanEdges.h>
#include <core/structs/learningMessages.h>

namespace PerformanceIO
{
class DataBuffer;
}

namespace KyoukoMind
{

struct CommonMessageData
{
    uint8_t type = UNDEFINED_MESSAGE;
    uint64_t messageId = 0;
    ClusterID targetClusterId = 0;
    uint8_t targetSite = 0;
    uint8_t requiredReply = 0;
};

class Message
{
public:
    Message(const ClusterID targetClusterId,
            const ClusterID sourceClusterId,
            const uint8_t targetSite);
    Message(const ClusterID targetClusterId,
            const uint8_t targetSite);
    Message(void *data, uint32_t size);
    ~Message();

    bool closeBuffer();

    CommonMessageData getMetaData() const;
    uint8_t getType() const;
    void setMetaData(const ClusterID targetClusterId,
                     const ClusterID sourceClusterId,
                     const uint32_t messageIdCounter,
                     const uint8_t targetSite);

    uint32_t getDataSize() const;
    void* getData() const;

    uint32_t getPayloadSize() const;
    uint32_t getNumberOfPayloadObj() const;
    void* getPayload() const;

protected:
    CommonMessageData m_metaData;
    PerformanceIO::DataBuffer* m_buffer = nullptr;
    uint32_t m_currentBufferPos = 0;
    uint32_t m_currentBufferSize = 0;

private:
    void initBuffer();
};

}

#endif // MESSAGE_H
