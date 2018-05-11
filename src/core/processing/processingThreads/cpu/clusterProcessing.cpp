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
ClusterProcessing::ClusterProcessing(NextChooser* nextChooser)
{
    m_nextChooser = nextChooser;
}


/**
 * @brief ClusterProcessing::initLearing
 * @param currentSection
 * @param weightDiff
 */
void ClusterProcessing::initLearing(KyoChanForwardEdgeSection *currentSection,
                                    const float weightDiff)
{

}

/**
 * @brief createNewEdgeForward
 * @param cluster
 * @param sourceEdgeClusterId
 * @param outgoBuffer
 * @param nextChooser
 */
void ClusterProcessing::createNewEdgeForward(EdgeCluster *cluster,
                                             const uint32_t sourceEdgeClusterId,
                                             const float weight,
                                             OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("createNewEdgeForward")
    const uint8_t nextSide = m_nextChooser->getNextCluster(cluster->getNeighbors(), 14);

    KyoChanLearingEdgeContainer newEdge;
    newEdge.sourceEdgeSectionId = sourceEdgeClusterId;
    newEdge.weight = weight;

    outgoBuffer->addLearingEdge(nextSide, &newEdge);
}

/**
 * @brief processEdgeForwardSection
 * @param currentSection
 * @param weight
 * @param outgoBuffer
 */
 void ClusterProcessing::processEdgeForwardSection(KyoChanForwardEdgeSection* currentSection,
                                                   const float weight,
                                                   OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processEdgeForwardSection")
    if(weight != 0.0)
    {
        if(weight > currentSection->totalWeight) {
            initLearing(currentSection,
                        weight - currentSection->totalWeight);
        }

        uint8_t sideCounter = 0;
        KyoChanForwardEdge* forwardEnd = currentSection->forwardEdges
                                         + currentSection->numberOfForwardEdges;
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
                currentSection->zeroPendingBit(sideCounter);
            }
            sideCounter++;
        }
    }

    if(currentSection->pendingEdges != 0)
    {
        for(uint8_t side = 0; side < 16; side++)
        {
            if(currentSection->isPendingBitSet(side))
            {
                KyoChanForwardEdgeContainer newEdge;
                newEdge.targetEdgeSectionId = 0xFFFFFFFF;
                newEdge.weight = currentSection->forwardEdges[side].weight;
                outgoBuffer->addForwardEdge(side, &newEdge);
                currentSection->zeroPendingBit(side);
            }
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
