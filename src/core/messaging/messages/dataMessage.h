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

    void addDirectEdge(const KyoChanDirectEdgeContainer *newEdge);
    void addForwardEdge(const KyoChanEdgeForwardContainer* newEdge);
    void addPendingEdge(const KyoChanPendingEdgeContainer* newEdge);
    void addAxonEdge(const KyoChanAxonEdgeContainer *newAxonEdge);
    void addNewEdge(const KyoChanLearingEdgeContainer* newEdge);
    void addNewEdgeReply(const KyoChanLearningEdgeReplyContainer* newEdgeReply);

private:
    void copyToBuffer(void* data, const uint8_t size);
    void checkBuffer(const uint8_t size);
};

}

#endif // DATAMESSAGE_H
