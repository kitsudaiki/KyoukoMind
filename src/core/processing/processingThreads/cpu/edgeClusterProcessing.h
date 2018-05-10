/**
 *  @file    edgeClusterProcessing.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef CLUSTERPROCESSING_H
#define CLUSTERPROCESSING_H

#include <common.h>
#include <core/structs/kyochanEdges.h>
#include <core/structs/kyochanNodes.h>
#include <core/structs/messageContainer.h>

#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>
#include <core/clustering/cluster/edgeCluster.h>

#include <core/messaging/messageController.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messages/replyMessage.h>

#include <core/messaging/messageQueues/outgoingMessageBuffer.h>
#include <core/processing/processingThreads/cpu/nextChooser.h>

namespace KyoukoMind
{
class OutgoingMessageBuffer;
class EdgeCluster;
class EdgeCluster;
class NodeCluster;
class NextChooser;

class EdgeClusterProcessing
{
public:
    EdgeClusterProcessing(NextChooser *nextChooser);

    bool processMessagesEdges(EdgeCluster* cluster);

protected:
    std::vector<uint8_t> m_sideOrder;
    NextChooser* m_nextChooser = nullptr;

    void processEdgeForwardSection(KyoChanForwardEdgeSection *currentSection,
                                   const float weight,
                                   OutgoingMessageBuffer *outgoBuffer);
    void processEdgeSection(KyoChanEdgeSection *currentSection,
                            const float weight,
                            KyoChanNode *nodes,
                            OutgoingMessageBuffer *outgoBuffer);
    void createNewEdgeForward(EdgeCluster *cluster,
                              const uint32_t sourceEdgeClusterId,
                              OutgoingMessageBuffer *outgoBuffer);
    void processAxonEdge(uint8_t *data,
                         OutgoingMessageBuffer *outgoBuffer);
    void processLerningEdge(uint8_t *data,
                            const uint8_t initSide,
                            EdgeCluster *cluster,
                            OutgoingMessageBuffer *outgoBuffer);
    void processPendingEdge(uint8_t *data,
                            EdgeCluster *cluster,
                            OutgoingMessageBuffer *outgoBuffer);
    void processForwardEdge(uint8_t *data,
                            EdgeCluster *cluster,
                            OutgoingMessageBuffer *outgoBuffer);
    void processLearningReply(uint8_t *data,
                              uint8_t initSide,
                              EdgeCluster *cluster);
    void processIncomDirectEdge(uint8_t *data,
                                EdgeCluster* cluster);
};

}

#endif // CLUSTERPROCESSING_H
