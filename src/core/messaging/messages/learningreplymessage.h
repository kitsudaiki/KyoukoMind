#ifndef LEARNINGREPLYMESSAGE_H
#define LEARNINGREPLYMESSAGE_H

#define MAX_NUMBER_OF_NEW_EDGES 100

#include "message.h"

namespace KyoukoMind
{

class LearningReplyMessage : public Message
{
public:
    LearningReplyMessage(const ClusterID targetClusterId,
                         const uint64_t messageId,
                         const uint8_t targetSite);
    LearningReplyMessage();

    bool convertFromByteArray(uint8_t* data);
    uint8_t* convertToByteArray();

    bool addNewEdgeReply(const KyoChanNewEdgeReply& newEdgeReply);
    uint8_t getNumberOfEdgeReplys() const;
    KyoChanNewEdgeReply* getNewEdgeReplys() const;

private:
    uint8_t m_maxNumberOfNewEdgeReplys = MAX_NUMBER_OF_NEW_EDGES;
    uint8_t m_numberOfNewEdgeReplys = 0;
    KyoChanNewEdgeReply m_newEdgeReplys[MAX_NUMBER_OF_NEW_EDGES];
};

}

#endif // LEARNINGREPLYMESSAGE_H
