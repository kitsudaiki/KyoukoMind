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
#include <core/processing/processingThreads/cpu/edgeClusterProcessing.h>

namespace KyoukoMind
{

MessageProcessing::MessageProcessing(EdgeClusterProcessing* clusterProcessing)
{
    m_clusterProcessing = clusterProcessing;
}

/**
 * @brief MessageProcessing::processStatusEdge
 * @param data
 * @param initialSide
 * @param cluster
 * @param outgoBuffer
 */
void MessageProcessing::processStatusEdge(uint8_t *data,
                                          const uint8_t initialSide,
                                          EdgeCluster *cluster,
                                          OutgoingMessageBuffer *outgoBuffer)
{
    std::cout<<"---"<<std::endl;
    std::cout<<"processStatusEdge"<<std::endl;
    KyoChanStatusEdgeContainer* edge = (KyoChanStatusEdgeContainer*)data;
    m_clusterProcessing->updateEdgeForwardSection(cluster,
                                                  edge->targetId,
                                                  edge->status,
                                                  initialSide,
                                                  outgoBuffer);
}

/**
 * @brief MessageProcessing::processInternalEdge
 * @param data
 * @param cluster
 */
void MessageProcessing::processInternalEdge(uint8_t *data,
                                            NodeCluster *cluster,
                                            OutgoingMessageBuffer *outgoBuffer)
{
    std::cout<<"---"<<std::endl;
    std::cout<<"processInternalEdge"<<std::endl;
    KyoChanInternalEdgeContainer* edge = (KyoChanInternalEdgeContainer*)data;
    m_clusterProcessing->processEdgeSection(cluster,
                                            edge->targetEdgeSectionId,
                                            edge->weight,
                                            outgoBuffer);
}

/**
 * @brief ClusterProcessing::processIncomDirectEdge
 * @param data
 * @param cluster
 */
void MessageProcessing::processDirectEdge(uint8_t *data,
                                          NodeCluster *cluster)
{
    std::cout<<"---"<<std::endl;
    std::cout<<"processDirectEdge"<<std::endl;
    KyoChanDirectEdgeContainer* edge = (KyoChanDirectEdgeContainer*)data;
    if(cluster->getClusterType() == NODE_CLUSTER) {
        ((NodeCluster*)cluster)->getNodeBlock()[edge->targetNodeId].currentState += edge->weight;
        std::cout<<"    node-id: "<<(int)edge->targetNodeId<<std::endl;
        std::cout<<"    current state: "<<((NodeCluster*)cluster)->getNodeBlock()[edge->targetNodeId].currentState<<std::endl;
    }
}

/**
 * @brief ClusterProcessing::processAxonEdge
 * @param data
 * @param cluster
 * @param outgoBuffer
 */
void MessageProcessing::processAxonEdge(uint8_t *data,
                                        const uint8_t initialSide,
                                        EdgeCluster* cluster,
                                        OutgoingMessageBuffer* outgoBuffer)
{
    std::cout<<"---"<<std::endl;
    std::cout<<"processAxonEdge"<<std::endl;
    KyoChanAxonEdgeContainer* edge = (KyoChanAxonEdgeContainer*)data;

    // check if target-cluster is reached
    if(edge->targetClusterPath != 0)
    {
        // if not reached update data
        uint8_t side = edge->targetClusterPath % 17;
        edge->targetClusterPath /= 17;

        // send edge to the next cluster
        outgoBuffer->addAxonEdge(side, edge);
    }
    else
    {
        // if target cluster reached, update the state of the target-axon with the edge
        m_clusterProcessing->processEdgeForwardSection(cluster,
                                                       edge->targetAxonId,
                                                       edge->weight,
                                                       initialSide,
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
                                           const uint8_t initialSide,
                                           EdgeCluster* cluster,
                                           OutgoingMessageBuffer* outgoBuffer)
{
    std::cout<<"---"<<std::endl;
    std::cout<<"processForwardEdge"<<std::endl;
    KyoChanForwardEdgeContainer* edge = (KyoChanForwardEdgeContainer*)data;

    if(edge->targetEdgeSectionId != UNINIT_STATE)
    {
        m_clusterProcessing->processEdgeForwardSection(cluster,
                                                       edge->targetEdgeSectionId,
                                                       edge->weight,
                                                       initialSide,
                                                       outgoBuffer);
    }
    else
    {
        uint32_t pendingEdgeId = cluster->getPendingForwardEdgeSectionId();
        if(pendingEdgeId != UNINIT_STATE) {
            m_clusterProcessing->processEdgeForwardSection(cluster,
                                                           pendingEdgeId,
                                                           edge->weight,
                                                           initialSide,
                                                           outgoBuffer);
        }
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
    std::cout<<"---"<<std::endl;
    std::cout<<"processLerningEdge"<<std::endl;
    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;

    const uint32_t targetEdgeSectionId = cluster->addEmptyForwardEdgeSection(initSide,
                                                                             edge->sourceEdgeSectionId);
    m_clusterProcessing->processEdgeForwardSection(cluster,
                                                   targetEdgeSectionId,
                                                   edge->weight,
                                                   initSide,
                                                   outgoBuffer);

    if(targetEdgeSectionId != UNINIT_STATE)
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
    std::cout<<"---"<<std::endl;
    std::cout<<"processLearningReply"<<std::endl;
    KyoChanLearningEdgeReplyContainer* edge = (KyoChanLearningEdgeReplyContainer*)data;

    KyoChanForwardEdgeSection* edgeForwardSections = ((EdgeCluster*)cluster)->getForwardEdgeSectionBlock();
    edgeForwardSections[edge->sourceEdgeSectionId].forwardEdges[initSide].targetId =
            edge->targetEdgeSectionId;
}

}
