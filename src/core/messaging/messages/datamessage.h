#ifndef DATAMESSAGE_H
#define DATAMESSAGE_H

#define MAX_NUMBER_OF_EDGES 100

#include "message.h"

namespace KyoukoMind
{

class DataMessage : public Message
{
public:
    DataMessage(const ClusterID targetClusterId,
                const ClusterID sourceClusterId,
                const uint32_t messageIdCounter,
                const uint8_t targetSite);
    DataMessage();

    bool convertFromByteArray(uint8_t* data);
    uint8_t* convertToByteArray();

    bool addEdge(const KyoChanEdge& newEdge);
    uint8_t getNumberOfEdges() const;
    KyoChanEdge* getEdges() const;

private:
    uint8_t m_maxNumberOfEdges = MAX_NUMBER_OF_EDGES;
    uint8_t m_numberOfEdges = 0;
    KyoChanEdge m_edges[MAX_NUMBER_OF_EDGES];
};

}

#endif // DATAMESSAGE_H
