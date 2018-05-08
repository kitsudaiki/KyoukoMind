/**
 *  @file    clusterProcessing.h
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

class ClusterProcessing
{
public:
    ClusterProcessing(NextChooser *nextChooser);

    bool processMessagesEdges(EdgeCluster* cluster);
    bool processAxons(EdgeCluster* cluster);

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
                         KyoChanAxon *axon,
                         OutgoingMessageBuffer *outgoBuffer);
    void processLerningEdge(uint8_t *data,
                            const uint8_t initSide,
                            EdgeCluster *cluster,
                            OutgoingMessageBuffer *outgoBuffer);
    void processPendingEdge(uint8_t *data,
                            EdgeCluster *cluster,
                            OutgoingMessageBuffer *outgoBuffer);


private:
    virtual void processForwardEdge(uint8_t *data,
                                    EdgeCluster* edgeCluster,
                                    OutgoingMessageBuffer* outgoBuffer) = 0;
    virtual void processLearningReply(uint8_t *data,
                                      uint8_t initSide,
                                      EdgeCluster* cluster) = 0;
    virtual void processIncomDirectEdge(uint8_t *data,
                                        EdgeCluster* cluster) = 0;
};

}

#endif // CLUSTERPROCESSING_H
