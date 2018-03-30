#ifndef CYCLEFINISHMESSAGE_H
#define CYCLEFINISHMESSAGE_H


#include "message.h"

namespace KyoukoMind
{

class CycleFinishMessage : public Message
{
public:
    CycleFinishMessage(const ClusterID targetClusterId,
                       const uint32_t messageId,
                       const uint8_t targetSite);
    CycleFinishMessage();

    bool convertFromByteArray(uint8_t* data);
    uint8_t* convertToByteArray();
};

}

#endif // CYCLEFINISHMESSAGE_H
