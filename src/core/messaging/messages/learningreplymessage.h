#ifndef LEARNINGREPLYMESSAGE_H
#define LEARNINGREPLYMESSAGE_H

#include "message.h"

class LearningReplyMessage : public Message
{
public:
    LearningReplyMessage(const quint32 messageId,
                         const quint8 site);
    LearningReplyMessage();

    bool convertFromByteArray(const QByteArray &data);
    QByteArray convertToByteArray();
};

#endif // LEARNINGREPLYMESSAGE_H
