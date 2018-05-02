/**
 *  @file    processingMethods.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef PROCESSINGMETHODS_H
#define PROCESSINGMETHODS_H

#include <common.h>
#include <core/structs/kyochanEdges.h>
#include <core/structs/kyochanNodes.h>
#include <core/structs/messageContainer.h>

#include <core/clustering/cluster/cluster.h>
#include <core/clustering/cluster/nodeCluster.h>
#include <core/clustering/cluster/edgeCluster.h>

#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

namespace KyoukoMind
{
class OutgoingMessageBuffer;
class Cluster;
class EdgeCluster;
class NodeCluster;
class NextChooser;


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
 * @brief processIncomLerningReplyEdge
 * @param data
 * @param initSide
 * @param cluster
 */
inline void processIncomLerningReplyEdge(uint8_t *data,
                                         uint8_t initSide,
                                         EdgeCluster* cluster)
{
    KyoChanLearningEdgeReplyContainer* edge = (KyoChanLearningEdgeReplyContainer*)data;

    KyoChanEdgeForwardSection* edgeForwardSections = cluster->getEdgeBlock();
    edgeForwardSections[edge->sourceEdgeSectionId].edgeForwards[initSide].targetEdgeSectionId = edge->targetEdgeSectionId;
}

/**
 * @brief processIncomLerningReplyEdgeOnNode
 * @param data
 * @param initSide
 * @param cluster
 */
inline void processIncomLerningReplyEdgeOnNode(uint8_t *data,
                                               uint8_t initSide,
                                               NodeCluster* cluster)
{
    KyoChanLearningEdgeReplyContainer* edge = (KyoChanLearningEdgeReplyContainer*)data;

    KyoChanEdgeSection* edgeSections = cluster->getEdgeBlock();
    edgeSections[edge->sourceEdgeSectionId].edgeForwards[initSide].targetEdgeSectionId = edge->targetEdgeSectionId;
}

}

#endif // PROCESSINGMETHODS_H
