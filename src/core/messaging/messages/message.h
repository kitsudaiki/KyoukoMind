#ifndef MESSAGE_H
#define MESSAGE_H

#include <QObject>
#include <QByteArray>
#include <common/structs.h>
#include <common/enums.h>


namespace KyoukoMind
{

struct CommonMessageData
{
    quint8 type = UNDEFINED;
    quint64 messageId = 0;
    quint8 site = 0;
    quint8 requiredReply = 0;
};

class Message
{
public:
    Message(const quint32 clusterId,
            const quint32 messageIdCounter,
            const quint8 site);
    Message(const quint64 messageId,
            const quint8 site);
    Message();

    virtual QByteArray convertToByteArray() = 0;

    CommonMessageData getMetaData() const;

protected:
    CommonMessageData m_metaData;

    QByteArray convertCommonToByteArray();
    quint32 convertCommonFromByteArray(const uint8_t *data);
};

}

#endif // MESSAGE_H
