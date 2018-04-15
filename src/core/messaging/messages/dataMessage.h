/**
 *  @file    dataMessage.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef DATAMESSAGE_H
#define DATAMESSAGE_H

#define MAX_NUMBER_OF_ENTRIES 100

#include "message.h"

namespace KyoukoMind
{

class DataMessage : public Message
{
public:
    DataMessage(const ClusterID targetClusterId,
                const ClusterID sourceClusterId,
                const uint8_t targetSite);
    DataMessage(void *data, uint32_t size);

    void addEdge(const KyoChanMessageEdge* newEdge);
    void addAxonEdge(const KyoChanAxonEdge *newAxonEdge);
    void addNewEdge(const KyoChanNewEdge* newEdge);
    void addNewEdgeReply(const KyoChanNewEdgeReply* newEdgeReply);

    uint8_t getNumberOfEntries() const;

private:
    void copyToBuffer(void* data);
    uint8_t m_maxNumberOfEntries = MAX_NUMBER_OF_ENTRIES;
    uint8_t m_numberOfEntries = 0;
    void checkBuffer();
};

}

#endif // DATAMESSAGE_H
