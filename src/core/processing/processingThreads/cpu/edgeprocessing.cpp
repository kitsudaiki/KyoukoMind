/**
 *  @file    edgeProcessing.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "edgeprocessing.h"

#include <core/clustering/cluster/cluster.h>
#include <core/clustering/cluster/emptyCluster.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/processing/processingThreads/cpu/nextChooser.h>

#include <core/messaging/messageController.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messages/replyMessage.h>

namespace KyoukoMind
{

EdgeProcessing::EdgeProcessing(NextChooser* nextChooser)
{
    m_sideOrder = {2,3,4,13,12,11};
    m_nextChooser = nextChooser;
}


/**
 * @brief CpuProcessingUnit::processIncomEdge
 * @param data
 * @return
 */
inline void processIncomEdge(uint8_t *data,
                             OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processIncomEdge")
    KyoChanEdgeContainer* edge = (KyoChanEdgeContainer*)data;

    // if not reached update data
    uint8_t side = edge->targetClusterPath % 16;
    edge->targetClusterPath /= 16;

    // send edge to the next cluster
    outgoBuffer->addEdge(side, edge);
}

/**
 * @brief CpuProcessingUnit::processIncomEdge
 * @param data
 * @return
 */
inline void processIncomEdgeOnNode(uint8_t *data,
                                   OutgoingMessageBuffer* outgoBuffer,
                                   KyoChanNode* node)
{
    OUTPUT("---")
    OUTPUT("processIncomEdgeOnNode")
    KyoChanEdgeContainer* edge = (KyoChanEdgeContainer*)data;
    std::cout<<"   edge->weight: "<< edge->weight<<std::endl;
    std::cout<<"   edge->targetClusterPath: "<< edge->targetClusterPath<<std::endl;

    // check if target-cluster is reached
    if(edge->targetClusterPath != 0)
    {
        // if not reached update data
        uint8_t side = edge->targetClusterPath % 16;
        edge->targetClusterPath /= 16;

        // send edge to the next cluster
        outgoBuffer->addEdge(side, edge);
    }
    else
    {
        // if target cluster reached, update the state of the target-node with the edge
        node[edge->targetNodeId].currentState += edge->weight;
    }
}

/**
 * @brief CpuProcessingUnit::processIncomAxonEdge
 * @param data
 * @return
 */
inline void processIncomAxonEdge(uint8_t *data,
                                 OutgoingMessageBuffer* outgoBuffer,
                                 KyoChanAxon* axon)
{
    OUTPUT("---")
    OUTPUT("processIncomAxonEdge")
    KyoChanAxonEdgeContainer* edge = (KyoChanAxonEdgeContainer*)data;

    std::cout<<"   edge->targetAxonId: "<<edge->targetAxonId<<std::endl;
    std::cout<<"   edge->weight: "<<edge->weight<<std::endl;
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
        axon[edge->targetAxonId].currentState = edge->weight;
    }
}

/**
 * @brief CpuProcessingUnit::processIncomPendingEdge
 * @param data
 * @param outgoBuffer
 */
inline void processIncomPendingEdge(uint8_t *data,
                                    OutgoingMessageBuffer *outgoBuffer,
                                    EdgeCluster* edgeCluster)
{
    OUTPUT("---")
    OUTPUT("processIncomPendingEdge")
    KyoChanPendingEdgeContainer* edge = (KyoChanPendingEdgeContainer*)data;
    const uint16_t numberOfPendingEdges = edgeCluster->getPendingEdges()->numberOfPendingEdges;

    // search in the current cluster for pending-edge-informations
    KyoChanPendingEdgeContainer* end = edgeCluster->getPendingEdges()->pendingEdges + numberOfPendingEdges;
    for(KyoChanPendingEdgeContainer* pendingEdge = edgeCluster->getPendingEdges()->pendingEdges;
        pendingEdge < end;
        pendingEdge++)
    {
        if(pendingEdge->newEdgeId == edge->newEdgeId)
        {
            // if pending-edge is found, get next side and to this direction
            outgoBuffer->addPendingEdge(pendingEdge->nextSite, edge);
            return;
        }
    }
}

/**
 * @brief EdgeProcessing::processIncomLerningEdge
 * @param data
 * @param initSide
 * @param outgoBuffer
 * @param edgeCluster
 */
inline void processIncomLerningEdgeOnNode(uint8_t *data,
                                          uint8_t initSide,
                                          OutgoingMessageBuffer* outgoBuffer,
                                          NodeCluster* nodeCluster,
                                          NextChooser* nextChooser)
{
    OUTPUT("---")
    OUTPUT("processIncomLerningEdgeOnNode")
    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;

    // check abort-condition
    if(edge->step > 8) {
        return;
    }

    if(rand() % 100 > POSSIBLE_NEXT_LEARNING_STEP)
    {
        // go to the next cluster

        // choose the next side and modify message
        uint8_t nextSide = nextChooser->getNextCluster(nodeCluster->getNeighbors(), initSide);
        edge->sourceClusterPath = (edge->sourceClusterPath << 16) + initSide;
        edge->step++;

        // send modified message to the next cluster
        outgoBuffer->addLearingEdge(nextSide, edge);

        // create pending edge
        KyoChanPendingEdgeContainer pendEdge;
        pendEdge.newEdgeId = edge->newEdgeId;
        pendEdge.nextSite = nextSide;

        // register pending edge in the current cluster
        nodeCluster->getPendingEdges()->addPendingEdges(pendEdge);
    }
    else
    {
        // stay in the current cluster

        // choose a node
        uint16_t nodeId = rand() % nodeCluster->getNumberOfNodes();
        nodeCluster->getNodeBlock()[nodeId].currentState += edge->weight;

        // create reply-message
        KyoChanLearningEdgeReplyContainer reply;
        reply.newEdgeId = edge->newEdgeId;
        reply.sourceAxonId = edge->sourceAxonId;
        reply.sourceClusterPath = edge->sourceClusterPath;
        reply.targetNodeId = nodeId;

        // send reply-message
        outgoBuffer->addLearningReplyMessage(initSide, &reply);
    }
}

/**
 * @brief EdgeProcessing::processIncomLerningEdge
 * @param data
 * @param initSide
 * @param outgoBuffer
 * @param edgeCluster
 */
inline void processIncomLerningEdge(uint8_t *data,
                                    uint8_t initSide,
                                    OutgoingMessageBuffer* outgoBuffer,
                                    EdgeCluster* edgeCluster,
                                    NextChooser* nextChooser)
{
    OUTPUT("---")
    OUTPUT("processIncomLerningEdge")
    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;

    // check abort-condition
    if(edge->step > 8) {
        return;
    }

    // choose the next side and modify message
    uint8_t nextSide = nextChooser->getNextCluster(edgeCluster->getNeighbors(), initSide);
    edge->sourceClusterPath = (edge->sourceClusterPath << 16) + initSide;
    edge->step++;

    // send modified message to the next cluster
    outgoBuffer->addLearingEdge(nextSide, edge);

    // create pending edge
    KyoChanPendingEdgeContainer pendEdge;
    pendEdge.newEdgeId = edge->newEdgeId;
    pendEdge.nextSite = nextSide;

    // register pending edge in the current cluster
    edgeCluster->getPendingEdges()->addPendingEdges(pendEdge);
}

/**
 * @brief EdgeProcessing::processIncomLerningReplyEdge
 * @param data
 * @param initSide
 * @param outgoBuffer
 * @param edgeCluster
 */
inline void processIncomLerningReplyEdge(uint8_t *data,
                                         uint8_t initSide,
                                         OutgoingMessageBuffer* outgoBuffer,
                                         EdgeCluster* edgeCluster)
{
    OUTPUT("---")
    OUTPUT("processIncomLerningReplyEdge")
    KyoChanLearningEdgeReplyContainer* edge = (KyoChanLearningEdgeReplyContainer*)data;

    // check if target-cluster is reached
    if(edge->sourceClusterPath != 0)
    {
        // if not reached update data
        edge->targetClusterPath = (edge->targetClusterPath << 16) + initSide;
        uint8_t side = edge->sourceClusterPath % 16;
        edge->sourceClusterPath /= 16;

        // send edge to the next cluster
        outgoBuffer->addLearningReplyMessage(side, edge);
    }
    else
    {
        // get pointer to the axon, which was the source of the learing process
        KyoChanAxon* axon = &(edgeCluster->getAxonBlock()[edge->sourceAxonId]);

        // search for the initial pending-edge in the axon
        KyoChanPendingEdgeContainer* end = edgeCluster->getPendingEdges()->pendingEdges + MAX_PENDING_EDGES;
        for(KyoChanPendingEdgeContainer* pendingEdge = edgeCluster->getPendingEdges()->pendingEdges;
            pendingEdge < end;
            pendingEdge++)
        {
            if(pendingEdge->newEdgeId == edge->newEdgeId)
            {
                // if found, then remove the entry
                pendingEdge->newEdgeId = 0;
                axon->numberOfPendingEdges--;

                // create a normal edge
                KyoChanEdge newEdge;
                newEdge.weight = pendingEdge->weight;
                newEdge.targetClusterPath = edge->targetClusterPath;
                newEdge.targetNodeId = edge->targetNodeId;

                // add the edge as replacement for the pending-edge
                edgeCluster->addEdge(edge->sourceAxonId, newEdge);
                return;
            }
        }
    }
}

/**
 * @brief EdgeProcessing::processInputMessages
 * @param nodeCluster
 * @return
 */
bool EdgeProcessing::processInputMessages(NodeCluster* nodeCluster)
{
    OUTPUT("---")
    OUTPUT("processInputMessages")
    IncomingMessageBuffer* incomBuffer = nodeCluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer();

    uint8_t* start = (uint8_t*)incomBuffer->getMessage(0)->getPayload();
    uint8_t* end = start + incomBuffer->getMessage(0)->getPayloadSize();
    for(uint8_t* data = start;
        data < end;
        data += 20)
    {
        OUTPUT("POI")
        processIncomEdgeOnNode(data, outgoBuffer, nodeCluster->getNodeBlock());
    }
    //incomBuffer->getMessage(0)->closeBuffer();
    //delete incomBuffer->getMessage(0);
    return true;
}

/**
 * @brief EdgeProcessing::processIncomingMessages
 * @param edgeCluster
 * @return
 */
bool EdgeProcessing::processIncomingMessages(EdgeCluster* edgeCluster)
{
    OUTPUT("---")
    OUTPUT("processIncomingMessages")
    if(edgeCluster == nullptr) {
        return false;
    }
    IncomingMessageBuffer* incomBuffer = edgeCluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = edgeCluster->getOutgoingMessageBuffer();

    const uint8_t type = edgeCluster->getClusterType();

    if(type == EDGE_CLUSTER)
    {
        // process normal communication
        for(uint8_t sidePos = 0; sidePos < m_sideOrder.size(); sidePos++)
        {
            uint8_t side = m_sideOrder[sidePos];
            uint8_t* start = (uint8_t*)incomBuffer->getMessage(side)->getPayload();
            uint8_t* end = start + incomBuffer->getMessage(side)->getPayloadSize();
            for(uint8_t* data = start;
                data < end;
                data += 20)
            {
                std::cout<<"   side: "<<(int)side<<std::endl;
                std::cout<<"   message-type: "<<(int)(*data)<<std::endl;
                switch((int)(*data))
                {
                    case EDGE_CONTAINER:
                        processIncomEdge(data, outgoBuffer);
                        break;
                    case AXON_EDGE_CONTAINER:
                        processIncomAxonEdge(data, outgoBuffer, edgeCluster->getAxonBlock());
                        break;
                    case PENDING_EDGE_CONTAINER:
                        processIncomPendingEdge(data, outgoBuffer, edgeCluster);
                        break;
                    case LEARNING_CONTAINER:
                        processIncomLerningEdge(data, side, outgoBuffer, edgeCluster, m_nextChooser);
                        break;
                    case LEARNING_REPLY_CONTAINER:
                        processIncomLerningReplyEdge(data, side, outgoBuffer, edgeCluster);
                        break;
                    default:
                        break;
                }

                //incomBuffer->getMessage(side)->closeBuffer();
                //delete incomBuffer->getMessage(side);
            }
        }
    }

    if(type == NODE_CLUSTER)
    {
        NodeCluster* nodeCluster = (NodeCluster*)edgeCluster;
        // process normal communication
        for(uint8_t sidePos = 0; sidePos < m_sideOrder.size(); sidePos++)
        {
            uint8_t side = m_sideOrder[sidePos];
            uint8_t* start = (uint8_t*)incomBuffer->getMessage(side)->getPayload();
            uint8_t* end = start + incomBuffer->getMessage(side)->getPayloadSize();
            for(uint8_t* data = start;
                data < end;
                data += 20)
            {
                switch((int)(data[0]))
                {
                    case EDGE_CONTAINER:
                        processIncomEdgeOnNode(data, outgoBuffer, nodeCluster->getNodeBlock());
                        break;
                    case AXON_EDGE_CONTAINER:
                        processIncomAxonEdge(data, outgoBuffer, edgeCluster->getAxonBlock());
                        break;
                    case PENDING_EDGE_CONTAINER:
                        processIncomPendingEdge(data, outgoBuffer, edgeCluster);
                        break;
                    case LEARNING_CONTAINER:
                        processIncomLerningEdgeOnNode(data, side, outgoBuffer, nodeCluster, m_nextChooser);
                        break;
                    case LEARNING_REPLY_CONTAINER:
                        processIncomLerningReplyEdge(data, side, outgoBuffer, edgeCluster);
                        break;
                    default:
                        break;
                }
            }
        }
    }


    return true;
}


}
