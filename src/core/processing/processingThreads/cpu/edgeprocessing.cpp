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
 * @brief processEdgeForwardSection
 * @param currentSection
 * @param weight
 * @param outgoBuffer
 */
inline void processEdgeForwardSection(KyoChanEdgeForwardSection* currentSection,
                                      float weight,
                                      OutgoingMessageBuffer* outgoBuffer)
{
    if(weight != 0.0)
    {
        uint8_t sideCounter;
        KyoChanEdgeForward* forwardEnd = currentSection->edgeForwards + 16;
        for(KyoChanEdgeForward* forwardEdge = currentSection->edgeForwards;
            forwardEdge < forwardEnd;
            forwardEdge++)
        {
            if(forwardEdge->weight != 0.0)
            {
                KyoChanEdgeForwardContainer newEdge;
                newEdge.targetEdgeSectionId = forwardEdge->targetEdgeSectionId;
                newEdge.weight = forwardEdge->weight;

                outgoBuffer->addEdge(sideCounter, &newEdge);
            }
            sideCounter++;
        }
    }
}

/**
 * @brief processEdgeSection
 * @param currentSection
 * @param weight
 * @param nodes
 * @param outgoBuffer
 */
inline void processEdgeSection(KyoChanEdgeSection* currentSection,
                               float weight,
                               KyoChanNode* nodes,
                               OutgoingMessageBuffer* outgoBuffer)
{
    if(weight != 0.0)
    {
        uint8_t sideCounter;
        KyoChanEdgeForward* forwardEnd = currentSection->edgeForwards + 16;
        for(KyoChanEdgeForward* forwardEdge = currentSection->edgeForwards;
            forwardEdge < forwardEnd;
            forwardEdge++)
        {
            if(forwardEdge->weight != 0.0)
            {
                KyoChanEdgeForwardContainer newEdge;
                newEdge.targetEdgeSectionId = forwardEdge->targetEdgeSectionId;
                newEdge.weight = forwardEdge->weight * weight;

                outgoBuffer->addEdge(sideCounter, &newEdge);
            }
            sideCounter++;
        }

        // process edge-section
        KyoChanEdge* end = currentSection->edges + currentSection->numberOfEdges;
        for(KyoChanEdge* edge = currentSection->edges;
            edge < end;
            edge++)
        {
            nodes[edge->targetNodeId].currentState += edge->weight * weight;
        }
    }
}

/**
 * @brief createNewEdgeForward
 * @param cluster
 * @param sourceEdgeClusterId
 * @param outgoBuffer
 * @param nextChooser
 */
inline void createNewEdgeForward(Cluster *cluster,
                                 const uint32_t sourceEdgeClusterId,
                                 OutgoingMessageBuffer* outgoBuffer,
                                 NextChooser *nextChooser)
{
    const uint8_t nextSide = nextChooser->getNextCluster(cluster->getNeighbors(), 14);
    const uint32_t newEdgeId = cluster->getNextNewEdgeId();

    // TODO: calculate a value
    const float weight = 100.0;

    KyoChanLearingEdgeContainer newEdge;
    newEdge.marker = newEdgeId;
    newEdge.sourceEdgeSectionId = sourceEdgeClusterId;
    newEdge.weight = weight;

    outgoBuffer->addLearingEdge(nextSide, &newEdge);
}

/**
 * @brief AxonProcessing::processAxons
 * @param cluster
 * @return
 */
bool EdgeProcessing::processAxons(Cluster* cluster)
{
    if(cluster == nullptr) {
        return false;
    }
    // get necessary values
    OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer();

    uint32_t axonId = 0;
    // process axons
    KyoChanAxon* axonEnd = cluster->getAxonBlock() + cluster->getNumberOfAxonBlocks();
    for(KyoChanAxon* axon = cluster->getAxonBlock();
        axon < axonEnd;
        axon++)
    {
        if(axon->currentState < AXON_PROCESS_BORDER) {
            continue;
        }

        // process normal edges
        if(cluster->getClusterType() == EDGE_CLUSTER)
        {
            EdgeCluster* edgeCluster = static_cast<EdgeCluster*>(cluster);
            KyoChanEdgeForwardSection* edgeForwardSections = edgeCluster->getEdgeBlock();
            processEdgeForwardSection(&edgeForwardSections[axon->edgeSectionId],
                                      axon->currentState,
                                      outgoBuffer);
        }
        if(cluster->getClusterType() == NODE_CLUSTER)
        {
            NodeCluster* nodeCluster = static_cast<NodeCluster*>(cluster);
            KyoChanEdgeSection* edgeSections = nodeCluster->getEdgeBlock();
            processEdgeSection(&edgeSections[axon->edgeSectionId],
                               axon->currentState,
                               nodeCluster->getNodeBlock(),
                               outgoBuffer);
        }

        axonId++;
    }
    return true;
}

/**
 * @brief processIncomForwardEdgeOnNode
 * @param data
 * @param nodeCluster
 * @param outgoBuffer
 */
inline void processIncomForwardEdgeOnNode(uint8_t *data,
                                          NodeCluster* nodeCluster,
                                          OutgoingMessageBuffer* outgoBuffer)
{
    KyoChanEdgeForwardContainer* edge = (KyoChanEdgeForwardContainer*)data;

    processEdgeSection(&nodeCluster->getEdgeBlock()[edge->targetEdgeSectionId],
                       edge->weight,
                       nodeCluster->getNodeBlock(),
                       outgoBuffer);
}

/**
 * @brief processIncomForwardEdge
 * @param data
 * @param nodeCluster
 * @param outgoBuffer
 */
inline void processIncomForwardEdge(uint8_t *data,
                                    EdgeCluster* edgeCluster,
                                    OutgoingMessageBuffer* outgoBuffer)
{
    KyoChanEdgeForwardContainer* edge = (KyoChanEdgeForwardContainer*)data;

    processEdgeForwardSection(&edgeCluster->getEdgeBlock()[edge->targetEdgeSectionId],
                              edge->weight,
                              outgoBuffer);
}

/**
 * @brief CpuProcessingUnit::processIncomAxonEdge
 * @param data
 * @return
 */
inline void processIncomAxonEdge(uint8_t *data,
                                 KyoChanAxon* axon,
                                 OutgoingMessageBuffer* outgoBuffer)
{
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
        axon[edge->targetAxonId].currentState = edge->weight;
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
                                    Cluster* cluster,
                                    OutgoingMessageBuffer* outgoBuffer)
{
    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;

    uint32_t targetEdgeSectionId = cluster->addEmptyEdgeSection(edge->marker);

    // create reply-message
    KyoChanLearningEdgeReplyContainer reply;
    reply.sourceEdgeSectionId = edge->sourceEdgeSectionId;
    reply.targetEdgeSectionId = targetEdgeSectionId;

    // send reply-message
    outgoBuffer->addLearningReplyMessage(initSide, &reply);
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
                                         Cluster* cluster)
{
    KyoChanLearningEdgeReplyContainer* edge = (KyoChanLearningEdgeReplyContainer*)data;

    if(cluster->getClusterType() == EDGE_CLUSTER)
    {
        EdgeCluster* edgeCluster = static_cast<EdgeCluster*>(cluster);
        KyoChanEdgeForwardSection* edgeForwardSections = edgeCluster->getEdgeBlock();
        edgeForwardSections[edge->sourceEdgeSectionId].edgeForwards[initSide].targetEdgeSectionId = edge->targetEdgeSectionId;

    }
    if(cluster->getClusterType() == NODE_CLUSTER)
    {
        NodeCluster* nodeCluster = static_cast<NodeCluster*>(cluster);
        KyoChanEdgeSection* edgeSections = nodeCluster->getEdgeBlock();
        edgeSections[edge->sourceEdgeSectionId].edgeForwards[initSide].targetEdgeSectionId = edge->targetEdgeSectionId;
    }
}

/**
 * @brief EdgeProcessing::processInputMessages
 * @param nodeCluster
 * @return
 */
bool EdgeProcessing::processInputMessages(NodeCluster* nodeCluster)
{

    IncomingMessageBuffer* incomBuffer = nodeCluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer();

    uint8_t* start = (uint8_t*)incomBuffer->getMessage(0)->getPayload();
    uint8_t* end = start + incomBuffer->getMessage(0)->getPayloadSize();
    for(uint8_t* data = start;
        data < end;
        data += data[1])
    {
        processIncomForwardEdgeOnNode(data, nodeCluster, outgoBuffer);
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
bool EdgeProcessing::processIncomingMessages(Cluster* cluster)
{
    if(cluster == nullptr) {
        return false;
    }

    IncomingMessageBuffer* incomBuffer = cluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer();

    const uint8_t type = cluster->getClusterType();

    // process normal communication
    for(uint8_t sidePos = 0; sidePos < m_sideOrder.size(); sidePos++)
    {
        uint8_t side = m_sideOrder[sidePos];
        uint8_t* start = (uint8_t*)incomBuffer->getMessage(side)->getPayload();
        uint8_t* end = start + incomBuffer->getMessage(side)->getPayloadSize();

        for(uint8_t* data = start;
            data < end;
            data += data[1])
        {
            switch((int)(*data))
            {
                case EDGE_FOREWARD_CONTAINER:
                    //processIncomForwardEdge(data, outgoBuffer);
                    break;
                case AXON_EDGE_CONTAINER:
                    processIncomAxonEdge(data, cluster->getAxonBlock(), outgoBuffer);
                    break;
                case LEARNING_CONTAINER:
                    //processIncomLerningEdge(data, side, outgoBuffer, cluster, m_nextChooser);
                    break;
                case LEARNING_REPLY_CONTAINER:
                    //processIncomLerningReplyEdge(data, side, outgoBuffer, cluster);
                    break;
                default:
                    break;
            }

            //incomBuffer->getMessage(side)->closeBuffer();
            //delete incomBuffer->getMessage(side);
        }
    }

    return true;
}

/**
 * @brief NodeProcessing::processNodes
 * @param nodeCluster
 * @return
 */
bool EdgeProcessing::processNodes(NodeCluster* nodeCluster)
{
    if(nodeCluster == nullptr) {
        return false;
    }
    // get necessary values
    OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer();
    const uint16_t numberOfNodes = nodeCluster->getNumberOfNodes();
    KyoChanAxon* axonBlock = nodeCluster->getAxonBlock();

    // process nodes
    KyoChanNode* end = nodeCluster->getNodeBlock() + numberOfNodes;
    for(KyoChanNode* nodes = nodeCluster->getNodeBlock();
        nodes < end;
        nodes++)
    {
        if(nodes->border <= nodes->currentState)
        {
            if(nodes->targetClusterPath != 0)
            {
                const uint8_t side = nodes->targetClusterPath % 16;
                // create new axon-edge
                KyoChanAxonEdgeContainer edge;
                edge.targetClusterPath = nodes->targetClusterPath / 16;
                edge.targetAxonId = nodes->targetAxonId;
                edge.weight = nodes->currentState;

                outgoBuffer->addAxonEdge(side, &edge);
            }
            else
            {
                axonBlock[nodes->targetAxonId].currentState = nodes->currentState;
            }

        }
        nodes->currentState /= NODE_COOLDOWN;
    }
    return true;
}

}
