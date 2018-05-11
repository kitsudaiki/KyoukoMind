/**
 *  @file    messsageProcessing.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "messageProcessing.h"

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
#include <core/processing/processingThreads/cpu/clusterProcessing.h>

namespace KyoukoMind
{

MessageProcessing::MessageProcessing(ClusterProcessing* clusterProcessing)
{
    m_clusterProcessing = clusterProcessing;
}

/**
 * @brief ClusterProcessing::processIncomDirectEdge
 * @param data
 * @param cluster
 */
void MessageProcessing::processDirectEdge(uint8_t *data,
                                          EdgeCluster *cluster)
{
    OUTPUT("---")
    OUTPUT("processDirectEdge")
    KyoChanDirectEdgeContainer* edge = (KyoChanDirectEdgeContainer*)data;
    if(cluster->getClusterType() == NODE_CLUSTER) {
        ((NodeCluster*)cluster)->getNodeBlock()[edge->targetNodeId].currentState += edge->weight;
    }
}

/**
 * @brief ClusterProcessing::processAxonEdge
 * @param data
 * @param cluster
 * @param outgoBuffer
 */
void MessageProcessing::processAxonEdge(uint8_t *data,
                                        EdgeCluster* cluster,
                                        OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processIncomAxonEdge")
    KyoChanAxonEdgeContainer* edge = (KyoChanAxonEdgeContainer*)data;

    // check if target-cluster is reached
    if(edge->targetClusterPath != 0)
    {
        // if not reached update data
        uint8_t side = edge->targetClusterPath % 16;
        edge->targetClusterPath /= 16;

        // send edge to the next cluster
        outgoBuffer->addAxonEdge(side, edge);
    }
    else
    {
        // if target cluster reached, update the state of the target-axon with the edge
        m_clusterProcessing->processEdgeForwardSection(&((cluster)->getForwardEdgeSectionBlock()[edge->targetAxonId]),
                                  edge->weight,
                                  outgoBuffer);
    }
}

/**
 * @brief processIncomForwardEdge
 * @param data
 * @param nodeCluster
 * @param outgoBuffer
 */
void MessageProcessing::processForwardEdge(uint8_t *data,
                                          EdgeCluster* cluster,
                                          OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processForwardEdge")
    KyoChanForwardEdgeContainer* edge = (KyoChanForwardEdgeContainer*)data;

    if(edge->targetEdgeSectionId != 0xFFFFFFFF)
    {
        m_clusterProcessing->processEdgeForwardSection(&((cluster)->getForwardEdgeSectionBlock()[edge->targetEdgeSectionId]),
                                  edge->weight,
                                  outgoBuffer);
    }
    else
    {
        KyoChanForwardEdgeSection* pendingEdge = cluster->getPendingForwardEdgeSectionBlock();
        m_clusterProcessing->processEdgeForwardSection(pendingEdge,
                                  edge->weight,
                                  outgoBuffer);
        cluster->decreaseNumberOfPendingForwardEdges();
    }
}

/**
 * @brief EdgeProcessing::processIncomLerningEdge
 * @param data
 * @param initSide
 * @param outgoBuffer
 * @param edgeCluster
 */
void MessageProcessing::processLerningEdge(uint8_t *data,
                                           const uint8_t initSide,
                                           EdgeCluster* cluster,
                                           OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processIncomLerningEdge")
    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;

    const uint32_t targetEdgeSectionId = cluster->addEmptyForwardEdgeSection();

    if(targetEdgeSectionId != 0xFFFFFFFF)
    {
        // create reply-message
        KyoChanLearningEdgeReplyContainer reply;
        reply.sourceEdgeSectionId = edge->sourceEdgeSectionId;
        reply.targetEdgeSectionId = targetEdgeSectionId;

        // send reply-message
        outgoBuffer->addLearningReplyMessage(initSide, &reply);
    }
}

/**
 * @brief processIncomLearningReply
 * @param data
 * @param initSide
 * @param cluster
 */
void MessageProcessing::processLearningReply(uint8_t *data,
                                             const uint8_t initSide,
                                             EdgeCluster* cluster)
{
    KyoChanLearningEdgeReplyContainer* edge = (KyoChanLearningEdgeReplyContainer*)data;

    KyoChanForwardEdgeSection* edgeForwardSections = ((EdgeCluster*)cluster)->getForwardEdgeSectionBlock();
    edgeForwardSections[edge->sourceEdgeSectionId].forwardEdges[initSide].targetId =
            edge->targetEdgeSectionId;
}

}
