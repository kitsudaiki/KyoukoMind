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
 * @brief ClusterProcessing::initLearing
 * @param currentSection
 * @param weightDiff
 */
void ClusterProcessing::initLearing(EdgeCluster *cluster,
                                    uint32_t forwardEdgeSectionId,
                                    const uint8_t inititalSide,
                                    const float weightDiff,
                                    OutgoingMessageBuffer* outgoBuffer)
{
    uint8_t loopCounter = 0;
    float weightCounter = weightDiff;
    KyoChanForwardEdgeSection* currentSection = &((cluster)->getForwardEdgeSectionBlock()[forwardEdgeSectionId]);

    while(weightCounter > 0 && loopCounter < MAX_LEARNING_LOOPS)
    {
        loopCounter++;

        if(inititalSide >= 14) {
            return;
        }
        const uint8_t nextSide = m_nextChooser->getNextCluster(cluster->getNeighbors(), inititalSide);
        float partitialWeight = randFloat(weightDiff);
        if(partitialWeight > weightCounter) {
            partitialWeight = weightCounter;
        }

        // if choose other cluster
        if(nextSide == 2 || nextSide == 3 | nextSide == 4 ||
                nextSide == 11 || nextSide == 12 || nextSide == 13)
        {
            if(currentSection->forwardEdges[nextSide].targetId == 0)
            {
                currentSection->forwardEdges[nextSide].targetId = SPECIAL_STATE;
                currentSection->forwardEdges[nextSide].weight = partitialWeight;

                KyoChanLearingEdgeContainer newEdge;
                newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
                newEdge.weight = partitialWeight;

                outgoBuffer->addLearingEdge(nextSide, &newEdge);
                currentSection->setPedingBit(nextSide);
            }
            else
            {
                currentSection->forwardEdges[nextSide].weight += partitialWeight;
            }
            weightCounter -= partitialWeight;
        }

        // if outgoing
        if(nextSide == 15 && currentSection->forwardEdges[nextSide].targetId == 0) {
            currentSection->forwardEdges[nextSide].weight += partitialWeight;
        }
        // if the same cluster
        if(nextSide == 1 && cluster->getClusterType() == NODE_CLUSTER)
        {
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
                                                   uint32_t forwardEdgeSectionId,
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
        uint8_t sideCounter = 0;
        KyoChanForwardEdge* forwardEnd = currentSection->forwardEdges + 16;
        for(KyoChanForwardEdge* forwardEdge = currentSection->forwardEdges;
            forwardEdge < forwardEnd;
            forwardEdge++)
        {
            if(forwardEdge->weight != 0.0)
            {
                KyoChanForwardEdgeContainer newEdge;
                newEdge.targetEdgeSectionId = forwardEdge->targetId;
                newEdge.weight = forwardEdge->weight * weight;
                outgoBuffer->addForwardEdge(sideCounter, &newEdge);

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
void ClusterProcessing::processEdgeSection(KyoChanEdgeSection* currentSection,
                                           const float weight,
                                           KyoChanNode* nodes)
{
    OUTPUT("---")
    OUTPUT("processEdgeSection")
    if(weight != 0.0)
    {
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

}
