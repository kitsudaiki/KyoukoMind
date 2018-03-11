#ifndef REPLYMESSAGE_H
#define REPLYMESSAGE_H

#include "message.h"

namespace KyoukoMind
{

class ReplyMessage : public Message
{
public:
    ReplyMessage(const quint32 messageId,
                 const quint8 site);
    ReplyMessage();

    bool convertFromByteArray(const QByteArray &data);
    QByteArray convertToByteArray();
};

}

#endif // REPLYMESSAGE_H
