/**
 *  @file    clusterProcessing.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "clusterProcessing.h"

#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

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

/**
 * @brief ClusterProcessing::ClusterProcessing
 * @param nextChooser
 */
ClusterProcessing::ClusterProcessing(NextChooser* nextChooser,
                                     PossibleKyoChanNodes* activeNodes)
{
    m_nextChooser = nextChooser;
    m_activeNodes = activeNodes;
}

/**
 * @brief ClusterProcessing::randFloat
 * @param b
 * @return
 */
inline float ClusterProcessing::randFloat(const float b)
{
    const float random = ((float) rand()) / (float) UNINIT_STATE;
    return random * b;
}

/**
 * @brief ClusterProcessing::updateEdgeForwardSection
 * @param cluster
 * @param forwardEdgeSectionId
 * @param status
 * @param inititalSide
 * @param outgoBuffer
 */
void ClusterProcessing::updateEdgeForwardSection(EdgeCluster *cluster,
                                                 const uint32_t forwardEdgeSectionId,
                                                 const float status,
                                                 const uint8_t inititalSide,
                                                 OutgoingMessageBuffer *outgoBuffer)
{
    KyoChanForwardEdgeSection* currentSection = &((cluster)->getForwardEdgeSectionBlock()[forwardEdgeSectionId]);
    //currentSection->forwardEdges[inititalSide].updateMemorize(status);

    if(currentSection->numberOfActiveEdges == 0 || currentSection->sourceId == 0) {
        return;
    }

    KyoChanStatusEdgeContainer newEdge;
    newEdge.status = status / currentSection->numberOfActiveEdges;
    newEdge.targetId = currentSection->sourceId;
    outgoBuffer->addStatusEdge(currentSection->sourceSide, &newEdge);
}

/**
 * @brief ClusterProcessing::learningForwardEdgeSection
 * @param currentSection
 * @param side
 * @param partitialWeight
 * @param outgoBuffer
 */
inline void ClusterProcessing::learningForwardEdgeSection(EdgeCluster* cluster,
                                                          KyoChanForwardEdgeSection* currentSection,
                                                          const uint32_t forwardEdgeSectionId,
                                                          const uint8_t inititalSide,
                                                          const float partitialWeight,
                                                          OutgoingMessageBuffer* outgoBuffer)
{
    // get side of learning-process
    const uint8_t nextSide = m_nextChooser->getNextCluster(cluster->getNeighbors(),
                                                           inititalSide,
                                                           cluster->getClusterType());

    // cluster-internal learning
    if(currentSection->forwardEdges[nextSide].targetId == UNINIT_STATE
            && nextSide == 8
            && cluster->getClusterType() == NODE_CLUSTER)
    {
        NodeCluster* nodeCluster = static_cast<NodeCluster*>(cluster);
        const uint32_t newSectionId = nodeCluster->addEmptyEdgeSection();
        currentSection->forwardEdges[nextSide].targetId = newSectionId;
    }

    // cluster-external lerning
    if(currentSection->forwardEdges[nextSide].targetId == UNINIT_STATE
            && nextSide != 8)
    {
        // send new learning-edge
        KyoChanLearingEdgeContainer newEdge;
        newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
        newEdge.weight = partitialWeight;
        outgoBuffer->addLearingEdge(nextSide, &newEdge);

        currentSection->setPedingBit(nextSide);
    }

    currentSection->updateWeight(nextSide, partitialWeight);
}

/**
  * @brief ClusterProcessing::processEdgeForwardSection
  * @param currentSection
  * @param weight
  * @param inititalSide
  * @param outgoBuffer
  */
 void ClusterProcessing::processEdgeForwardSection(EdgeCluster *cluster,
                                                   const uint32_t forwardEdgeSectionId,
                                                   const float weight,
                                                   const uint8_t inititalSide,
                                                   OutgoingMessageBuffer* outgoBuffer)
{
    KyoChanForwardEdgeSection* currentSection = &((cluster)->getForwardEdgeSectionBlock()[forwardEdgeSectionId]);

    if(weight != 0.0 || currentSection->pendingEdges != 0)
    {
        // learning
        if(weight > currentSection->totalWeight) {
            learningForwardEdgeSection(cluster,
                                       currentSection,
                                       forwardEdgeSectionId,
                                       inititalSide,
                                       weight - currentSection->totalWeight,
                                       outgoBuffer);
        }

        // normal processing
        uint8_t sideCounter = 2;
        KyoChanForwardEdge* forwardStart = currentSection->forwardEdges + sideCounter;
        KyoChanForwardEdge* forwardEnd = currentSection->forwardEdges + 17;
        for(KyoChanForwardEdge* forwardEdge = forwardStart;
            forwardEdge < forwardEnd;
            forwardEdge++)
        {
            if(forwardEdge->weight != 0.0)
            {
                if(sideCounter == 8 && forwardEdge->targetId != UNINIT_STATE) {
                    KyoChanInternalEdgeContainer newEdge;
                    newEdge.targetEdgeSectionId = forwardEdge->targetId;
                    newEdge.weight = forwardEdge->weight * weight;
                    outgoBuffer->addInternalEdge(sideCounter, &newEdge);
                }
                if(sideCounter <= 14 && sideCounter != 8) {
                    KyoChanForwardEdgeContainer newEdge;
                    newEdge.targetEdgeSectionId = forwardEdge->targetId;
                    newEdge.weight = forwardEdge->weight * weight;
                    outgoBuffer->addForwardEdge(sideCounter, &newEdge);
                }
                if(sideCounter == 16) {
                    KyoChanDirectEdgeContainer newEdge;
                    newEdge.targetNodeId = (int16_t)forwardEdge->targetId;
                    newEdge.weight = forwardEdge->weight * weight;
                    outgoBuffer->addDirectEdge(sideCounter, &newEdge);
                }
            }

            forwardEdge->weight *= forwardEdge->memorize;

            currentSection->zeroPendingBit(sideCounter);
            sideCounter++;
        }
    }
}

 /**
  * @brief ClusterProcessing::learningInternalForwardEdge
  * @param currentSection
  * @param cluster
  * @param partitialWeight
  */
 inline void ClusterProcessing::learningEdgeSection(KyoChanEdgeSection* currentSection,
                                                    const float partitialWeight)
 {
     uint16_t chooseOfExist = rand() % currentSection->numberOfEdges + OVERPROVISIONING;
     if(chooseOfExist >= currentSection->numberOfEdges)
     {
         KyoChanEdge newEdge;
         newEdge.targetNodeId = rand() % m_activeNodes->numberOfActiveNodes;
         newEdge.weight = partitialWeight;
         currentSection->addEdge(newEdge);
     }
 }

 /**
 * @brief ClusterProcessing::processEdgeSection
 * @param cluster
 * @param edgeSectionId
 * @param weight
 * @param outgoBuffer
 */
void ClusterProcessing::processEdgeSection(NodeCluster *cluster,
                                           uint32_t edgeSectionId,
                                           const float weight,
                                           OutgoingMessageBuffer* outgoBuffer)
{
    if(weight != 0.0)
    {
        KyoChanEdgeSection* currentSection = &((cluster)->getEdgeSectionBlock()[edgeSectionId]);

        // learning
        if(weight > currentSection->totalWeight) {
            learningEdgeSection(currentSection,
                                weight - currentSection->totalWeight);
        }

        // process edge-section
        KyoChanNode* nodes = cluster->getNodeBlock();
        KyoChanEdge* end = currentSection->edges + currentSection->numberOfEdges;

        float updateValue = 0;

        for(KyoChanEdge* edge = currentSection->edges;
            edge < end;
            edge++)
        {
            // update node with the edge-weight
            nodes[edge->targetNodeId].currentState += edge->weight * weight;

            // update memorize-value
            if(nodes[edge->targetNodeId].border
                    <= nodes[edge->targetNodeId].currentState * NODE_COOLDOWN)
            {
                edge->memorize += (1.0f - edge->memorize) / EDGE_MEMORIZE_UPDATE;
            } else {
                edge->memorize -= (1.0f - edge->memorize) / EDGE_MEMORIZE_UPDATE;
            }

            // memorize the current edge-weight
            edge->weight *= edge->memorize;
        }

        //KyoChanStatusEdgeContainer newEdge;
        //newEdge.status = updateValue;
        //newEdge.targetId = currentSection->sourceId;
        //outgoBuffer->addStatusEdge(8, &newEdge);
    }
}

}
