#ifndef DATAMESSAGE_H
#define DATAMESSAGE_H

#define MAX_NUMBER_OF_EDGES 100

#include "message.h"

namespace KyoukoMind
{

class DataMessage : public Message
{
public:
    DataMessage(const quint32 clusterId,
                const quint32 messageIdCounter,
                const quint8 site);
    DataMessage();

    bool convertFromByteArray(const QByteArray &data);
    QByteArray convertToByteArray();

    bool addEdge(const KyoChanEdge& newEdge);
    quint8 getNumberOfEdges() const;
    KyoChanEdge* getEdges() const;

private:
    quint8 m_maxNumberOfEdges = MAX_NUMBER_OF_EDGES;
    quint8 m_numberOfEdges = 0;
    KyoChanEdge m_edges[MAX_NUMBER_OF_EDGES];
};

}

#endif // DATAMESSAGE_H
