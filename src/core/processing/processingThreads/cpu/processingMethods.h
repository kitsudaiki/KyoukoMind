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
inline void processEdgeForwardSection(KyoChanForwardEdgeSection* currentSection,
                                      float weight,
                                      OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processEdgeForwardSection")
    if(weight != 0.0)
    {
        uint8_t sideCounter;
        KyoChanForwardEdge* forwardEnd = currentSection->forwardEdges
                + currentSection->numberOfForwardEdges;
        for(KyoChanForwardEdge* forwardEdge = currentSection->forwardEdges;
            forwardEdge < forwardEnd;
            forwardEdge++)
        {
            std::cout<<"        side: "<<(int)sideCounter<<std::endl;
            std::cout<<"        forwardEdge->weight: "<<forwardEdge->weight<<std::endl;
            if(forwardEdge->weight != 0.0)
            {
                KyoChanEdgeForwardContainer newEdge;
                newEdge.targetEdgeSectionId = forwardEdge->targetEdgeSectionId;
                newEdge.weight = forwardEdge->weight;

                outgoBuffer->addForwardEdge(sideCounter, &newEdge);
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
    OUTPUT("---")
    OUTPUT("processEdgeSection")
    std::cout<<"    weight: "<<weight<<std::endl;
    if(weight != 0.0)
    {
        uint8_t sideCounter;
        KyoChanForwardEdge* forwardEnd = currentSection->forwardEdges
                + currentSection->numberOfForwardEdges;
        for(KyoChanForwardEdge* forwardEdge = currentSection->forwardEdges;
            forwardEdge < forwardEnd;
            forwardEdge++)
        {
            std::cout<<"        side: "<<(int)sideCounter<<std::endl;
            std::cout<<"        forwardEdge->weight: "<<forwardEdge->weight<<std::endl;
            if(forwardEdge->weight != 0.0)
            {
                KyoChanEdgeForwardContainer newEdge;
                newEdge.targetEdgeSectionId = forwardEdge->targetEdgeSectionId;
                newEdge.weight = forwardEdge->weight * weight;

                outgoBuffer->addForwardEdge(sideCounter, &newEdge);
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
    OUTPUT("---")
    OUTPUT("createNewEdgeForward")
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
 * @brief CpuProcessingUnit::processIncomAxonEdge
 * @param data
 * @return
 */
inline void processAxonEdge(uint8_t *data,
                            KyoChanAxon* axon,
                            OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processIncomAxonEdge")
    KyoChanAxonEdgeContainer* edge = (KyoChanAxonEdgeContainer*)data;

    // check if target-cluster is reached
    std::cout<<"    edge->targetClusterPath: "<<edge->targetClusterPath<<std::endl;
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
inline void processLerningEdge(uint8_t *data,
                               uint8_t initSide,
                               Cluster* cluster,
                               OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processIncomLerningEdge")
    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;

    const uint32_t targetEdgeSectionId = cluster->addEmptyEdgeSection();

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
 * @brief processPendingEdge
 * @param data
 * @param cluster
 * @param outgoBuffer
 */
inline void processPendingEdge(uint8_t *data,
                               Cluster* cluster,
                               OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processPendingEdge")
    KyoChanPendingEdgeContainer* edge = (KyoChanPendingEdgeContainer*)data;

}

}

#endif // PROCESSINGMETHODS_H
