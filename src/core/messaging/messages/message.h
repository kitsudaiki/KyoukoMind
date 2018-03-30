#ifndef MESSAGE_H
#define MESSAGE_H

#include <common.h>

namespace KyoukoMind
{

struct CommonMessageData
{
    uint8_t type = UNDEFINED;
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
            const uint32_t messageIdCounter,
            const uint8_t targetSite);
    Message(const ClusterID targetClusterId,
            const uint64_t messageId,
            const uint8_t targetSite);
    Message();

    virtual uint8_t* convertToByteArray() = 0;

    CommonMessageData getMetaData() const;
    uint8_t getType() const;
    void setMetaData(const ClusterID targetClusterId,
                     const ClusterID sourceClusterId,
                     const uint32_t messageIdCounter,
                     const uint8_t targetSite);

protected:
    CommonMessageData m_metaData;

    uint8_t* convertCommonToByteArray(const uint32_t size);
    uint32_t convertCommonFromByteArray(const uint8_t *data);
};

}

#endif // MESSAGE_H
