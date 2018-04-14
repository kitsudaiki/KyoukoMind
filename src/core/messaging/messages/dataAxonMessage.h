/**
 *  @file    dataAxonMessage.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef DATAAXONMESSAGE_H
#define DATAAXONMESSAGE_H

#define MAX_NUMBER_OF_EDGES 100

#include "message.h"

namespace KyoukoMind
{

class DataAxonMessage : public Message
{
public:
    DataAxonMessage(const ClusterID targetClusterId,
                    const ClusterID sourceClusterId,
                    const uint32_t messageIdCounter,
                    const uint8_t targetSite);
    DataAxonMessage();

    bool convertFromByteArray(uint8_t* data);
    uint8_t* convertToByteArray();

    bool addAxonEdge(const KyoChanAxonEdge &newAxonEdge);
    uint8_t getNumberOfAxonEdges() const;
    KyoChanAxonEdge* getAxonEdges() const;

private:
    uint8_t m_maxNumberOfAxonEdges = MAX_NUMBER_OF_EDGES;
    uint8_t m_numberOfAxonEdges = 0;
    KyoChanAxonEdge m_edges[MAX_NUMBER_OF_EDGES];
};

}

#endif // DATAAXONMESSAGE_H
