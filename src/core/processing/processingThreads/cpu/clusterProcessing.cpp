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
    const float random = ((float) rand()) / (float) SPECIAL_STATE;
    return random * b;
}

/**
 * @brief ClusterProcessing::learningExternalForwardEdge
 * @param currentSection
 * @param side
 * @param partitialWeight
 * @param outgoBuffer
 */
inline void ClusterProcessing::learningExternalForwardEdge(KyoChanForwardEdgeSection* currentSection,
                                                           const uint32_t forwardEdgeSectionId,
                                                           const uint8_t side,
                                                           const float partitialWeight,
                                                           OutgoingMessageBuffer* outgoBuffer)
{
    if(currentSection->forwardEdges[side].targetId == 0)
    {
        currentSection->forwardEdges[side].targetId = SPECIAL_STATE;

        // send new learning-edge
        KyoChanLearingEdgeContainer newEdge;
        newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
        newEdge.weight = partitialWeight;
        outgoBuffer->addLearingEdge(side, &newEdge);
        currentSection->setPedingBit(side);
    }

    currentSection->updateWeight(side, partitialWeight);
}

/**
 * @brief ClusterProcessing::learningInternalForwardEdge
 * @param currentSection
 * @param cluster
 * @param partitialWeight
 */
inline void ClusterProcessing::learningInternalForwardEdge(KyoChanForwardEdgeSection* currentSection,
                                                           EdgeCluster *cluster,
                                                           const float partitialWeight)
{
    NodeCluster* nodeCluster = (NodeCluster*)cluster;

    // add new edge-section if necessary
    if(currentSection->forwardEdges[1].targetId == 0) {
        uint32_t newEdgeSectionId = nodeCluster->addEmptyEdgeSection();
        currentSection->forwardEdges[1].targetId = newEdgeSectionId;
    }
    currentSection->updateWeight(1, partitialWeight);

    KyoChanEdgeSection* edgeSection = &(nodeCluster->getEdgeSectionBlock()[currentSection->forwardEdges[1].targetId]);

    uint16_t chooseOfExist = rand() % edgeSection->numberOfEdges + OVERPROVISIONING;
    if(chooseOfExist >= edgeSection->numberOfEdges)
    {
        KyoChanEdge newEdge;
        newEdge.targetNodeId = rand() % m_activeNodes->numberOfActiveNodes;
        newEdge.weight = 1.0;  // TODO
        edgeSection->addEdge(newEdge);
    }


}

/**
 * @brief ClusterProcessing::initLearing
 * @param currentSection
 * @param weightDiff
 */
void ClusterProcessing::initLearing(EdgeCluster *cluster,
                                    const uint32_t forwardEdgeSectionId,
                                    const uint8_t inititalSide,
                                    const float weightDiff,
                                    OutgoingMessageBuffer* outgoBuffer)
{
    uint8_t loopCounter = 0;
    float weightCounter = weightDiff;
    KyoChanForwardEdgeSection* currentSection = &((cluster)->getForwardEdgeSectionBlock()[forwardEdgeSectionId]);


    if(inititalSide >= 14) {
        return;
    }

    while(weightCounter > 0 && loopCounter < MAX_LEARNING_LOOPS)
    {
        loopCounter++;

        // choose next-side
        const uint8_t nextSide = m_nextChooser->getNextCluster(cluster->getNeighbors(), inititalSide);

        // choose weight-part
        float partitialWeight = randFloat(weightDiff);
        if(partitialWeight > weightCounter) {
            partitialWeight = weightCounter;
        }

        // if choose other cluster
        if(nextSide == 2 || nextSide == 3 | nextSide == 4 ||
                nextSide == 11 || nextSide == 12 || nextSide == 13) {
            learningExternalForwardEdge(currentSection,
                                        forwardEdgeSectionId,
                                        nextSide,
                                        partitialWeight,
                                        outgoBuffer);
            weightCounter -= partitialWeight;
            continue;
        }

        // if outgoing
        if(nextSide == 15) {
            if(currentSection->updateWeight(nextSide, partitialWeight)) {
                weightCounter -= partitialWeight;
            }
            continue;
        }
        // if the same cluster
        if(nextSide == 1
                && m_activeNodes->numberOfActiveNodes > 0
                && cluster->getClusterType() == NODE_CLUSTER) {
            learningInternalForwardEdge(currentSection,
                                        cluster,
                                        partitialWeight);
            weightCounter -= partitialWeight;
            continue;
        }
    }
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
    OUTPUT("---")
    OUTPUT("processEdgeForwardSection")
    KyoChanForwardEdgeSection* currentSection = &((cluster)->getForwardEdgeSectionBlock()[forwardEdgeSectionId]);

    if(weight != 0.0 || currentSection->pendingEdges != 0)
    {
        // learning
        if(weight > currentSection->totalWeight) {
            initLearing(cluster,
                        forwardEdgeSectionId,
                        inititalSide,
                        weight - currentSection->totalWeight,
                        outgoBuffer);
        }

        // normal processing
        uint8_t sideCounter = 2;
        KyoChanForwardEdge* forwardStart = currentSection->forwardEdges + sideCounter;
        KyoChanForwardEdge* forwardEnd = currentSection->forwardEdges + 16;
        for(KyoChanForwardEdge* forwardEdge = forwardStart;
            forwardEdge < forwardEnd;
            forwardEdge++)
        {
            if(forwardEdge->weight != 0.0)
            {
                if(sideCounter <= 13) {
                    KyoChanForwardEdgeContainer newEdge;
                    newEdge.targetEdgeSectionId = forwardEdge->targetId;
                    newEdge.weight = forwardEdge->weight * weight;
                    outgoBuffer->addForwardEdge(sideCounter, &newEdge);
                }
                if(sideCounter == 14) {
                    KyoChanInternalEdgeContainer newEdge;
                    newEdge.targetEdgeSectionId = forwardEdge->targetId;
                    newEdge.weight = forwardEdge->weight * weight;
                    outgoBuffer->addInternalEdge(sideCounter, &newEdge);
                }
                if(sideCounter == 15) {
                    KyoChanDirectEdgeContainer newEdge;
                    newEdge.targetNodeId = (int16_t)forwardEdge->targetId;
                    newEdge.weight = forwardEdge->weight * weight;
                    outgoBuffer->addDirectEdge(sideCounter, &newEdge);
                }
            }
            currentSection->zeroPendingBit(sideCounter);
            sideCounter++;
        }
    }
}

/**
 * @brief processEdgeSection
 * @param currentSection
 * @param weight
 * @param nodes
 */
void ClusterProcessing::processEdgeSection(NodeCluster *cluster,
                                           uint32_t edgeSectionId,
                                           const float weight)
{
    OUTPUT("---")
    OUTPUT("processEdgeSection")
    if(weight != 0.0)
    {
        // process edge-section
        KyoChanEdgeSection* currentSection = &(cluster->getEdgeSectionBlock()[edgeSectionId]);
        KyoChanNode* nodes = cluster->getNodeBlock();
        KyoChanEdge* end = currentSection->edges + currentSection->numberOfEdges;

        for(KyoChanEdge* edge = currentSection->edges;
            edge < end;
            edge++)
        {
            nodes[edge->targetNodeId].currentState += edge->weight * weight;
        }
    }
}

}
