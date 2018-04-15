/**
 *  @file    replyMessage.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef REPLYMESSAGE_H
#define REPLYMESSAGE_H

#include "message.h"

namespace KyoukoMind
{

class ReplyMessage : public Message
{
public:
    ReplyMessage(const ClusterID targetClusterId,
                 const uint8_t targetSite);
    ReplyMessage(void *data, uint32_t size);
};

}

#endif // REPLYMESSAGE_H
