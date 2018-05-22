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

    void addStatusEdge(const KyoChanStatusEdgeContainer* newEdge);
    void addInternalEdge(const KyoChanInternalEdgeContainer *newEdge);
    void addDirectEdge(const KyoChanDirectEdgeContainer *newEdge);
    void addForwardEdge(const KyoChanForwardEdgeContainer* newEdge);
    void addAxonEdge(const KyoChanAxonEdgeContainer *newAxonEdge);
    void addLearningEdge(const KyoChanLearingEdgeContainer* newEdge);
    void addLearningReplyEdgeReply(const KyoChanLearningEdgeReplyContainer* newEdgeReply);
    void addDeleteEdge(const KyoChanDeleteContainer *newEdge);

private:
    void copyToBuffer(void* data, const uint8_t size);
    void checkBuffer(const uint8_t size);
};

}

#endif // DATAMESSAGE_H
