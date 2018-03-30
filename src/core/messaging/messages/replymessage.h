#ifndef REPLYMESSAGE_H
#define REPLYMESSAGE_H

#include "message.h"

namespace KyoukoMind
{

class ReplyMessage : public Message
{
public:
    ReplyMessage(const ClusterID targetClusterId,
                 const uint32_t messageId,
                 const uint8_t targetSite);
    ReplyMessage();

    bool convertFromByteArray(uint8_t* data);
    uint8_t* convertToByteArray();
};

}

#endif // REPLYMESSAGE_H
