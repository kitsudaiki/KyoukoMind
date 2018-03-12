#ifndef LEARNINGREPLYMESSAGE_H
#define LEARNINGREPLYMESSAGE_H

#define MAX_NUMBER_OF_NEW_EDGES 100

#include "message.h"

namespace KyoukoMind
{

class LearningReplyMessage : public Message
{
public:
    LearningReplyMessage(const quint32 clusterId,
                         const quint32 messageIdCounter,
                         const quint8 site);
    LearningReplyMessage();

    bool convertFromByteArray(const QByteArray &data);
    QByteArray convertToByteArray();

    bool addNewEdgeReply(const KyoChanNewEdgeReply& newEdgeReply);
    quint8 getNumberOfEdgeReplys() const;
    KyoChanNewEdgeReply* getNewEdgeReplys() const;

private:
    quint8 m_maxNumberOfNewEdgeReplys = MAX_NUMBER_OF_NEW_EDGES;
    quint8 m_numberOfNewEdgeReplys = 0;
    KyoChanNewEdgeReply m_newEdgeReplys[MAX_NUMBER_OF_NEW_EDGES];
};

}

#endif // LEARNINGREPLYMESSAGE_H
