#ifndef LEARNINGMESSAGE_H
#define LEARNINGMESSAGE_H

#define MAX_NUMBER_OF_NEW_EDGES 100

#include "message.h"

namespace KyoukoMind
{

class LearningMessage : public Message
{
public:
    LearningMessage(const quint32 clusterId,
                    const quint32 messageIdCounter,
                    const quint8 site);
    LearningMessage();

    bool convertFromByteArray(const QByteArray &data);
    QByteArray convertToByteArray();

    bool addNewEdge(const KyoChanNewEdge& newEdge);
    quint8 getNumberOfEdges() const;
    KyoChanNewEdge* getNewEdges() const;

private:
    quint8 m_maxNumberOfNewEdges = MAX_NUMBER_OF_NEW_EDGES;
    quint8 m_numberOfNewEdges = 0;
    KyoChanNewEdge m_newEdges[MAX_NUMBER_OF_NEW_EDGES];
};

}

#endif // LEARNINGMESSAGE_H
