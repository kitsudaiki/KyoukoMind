/**
 *  @file    axonProcessing.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "axonprocessing.h"
#include <core/processing/processingThreads/cpu/nextChooser.h>

#include <core/cluster/cluster.h>
#include <core/cluster/emptyCluster.h>
#include <core/cluster/edgeCluster.h>
#include <core/cluster/nodeCluster.h>

#include <core/messaging/messageQueues/messageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

namespace KyoukoMind
{

AxonProcessing::AxonProcessing(NextChooser *nextChooser)
{
    m_nextChooser = nextChooser;
}

inline void processEdgeSection(KyoChanEdgeSection* currentSection,
                               OutgoingMessageBuffer* outgoBuffer)
{
    // process edge-section
    for(KyoChanEdge* edge = currentSection->edges;
        edge < edge + currentSection->numberOfEdges;
        edge++)
    {
        if(edge->targetClusterPath != 0) {
            uint8_t side = edge->targetClusterPath % 16;

            KyoChanEdgeContainer newEdge;
            newEdge.weight = edge->weight;
            newEdge.targetNodeId = edge->targetNodeId;
            newEdge.targetClusterPath = edge->targetClusterPath / 16;

            outgoBuffer->addEdge(side, &newEdge);
        }
    }
}

inline void processEdgeSectionOnNode(KyoChanEdgeSection* currentSection,
                                     OutgoingMessageBuffer* outgoBuffer,
                                     KyoChanNode* nodes)
{
    // process edge-section
    for(KyoChanEdge* edge = currentSection->edges;
        edge < edge + currentSection->numberOfEdges;
        edge++)
    {
        if(edge->targetClusterPath != 0) {
            uint8_t side = edge->targetClusterPath % 16;

            KyoChanEdgeContainer newEdge;
            newEdge.weight = edge->weight;
            newEdge.targetNodeId = edge->targetNodeId;
            newEdge.targetClusterPath = edge->targetClusterPath / 16;

            outgoBuffer->addEdge(side, &newEdge);
        }
        else
        {
            nodes[edge->targetNodeId].currentState =+ edge->weight;
        }
    }
}

/**
 * @brief CpuProcessingUnit::createNewEdge
 * @param axon
 * @param axonId
 */
inline void createNewEdge(EdgeCluster *edgeCluster,
                          const uint32_t axonId,
                          OutgoingMessageBuffer* outgoBuffer,
                          NextChooser *nextChooser)
{
    uint8_t nextSide = nextChooser->getNextCluster(edgeCluster->getNeighbors(), 14);
    KyoChanLearingEdgeContainer newEdge;
    newEdge.newEdgeId = edgeCluster->getNextNewEdgeId();
    newEdge.sourceAxonId = axonId;
    newEdge.weight = 1.0;
    // TODO: finish
    outgoBuffer->addLearingEdge(nextSide, &newEdge);
}

/**
 * @brief AxonProcessing::processPendingEdges
 * @param axon
 * @param outgoBuffer
 */
inline void processPendingEdges(KyoChanAxon* axon,
                                OutgoingMessageBuffer* outgoBuffer)
{
    for(KyoChanPendingEdgeContainer* pendingEdge = axon->pendingEdges.pendingEdges;
        pendingEdge < pendingEdge + MAX_PENDING_EDGES;
        pendingEdge++)
    {
        if(pendingEdge->newEdgeId != 0)
        {
            uint8_t side = pendingEdge->targetClusterPath % 16;

            KyoChanEdgeContainer newEdge;
            newEdge.weight = pendingEdge->weight;

            outgoBuffer->addEdge(side, &newEdge);
        }
    }
}

/**
 * @brief AxonProcessing::processAxons
 * @param edgeCluster
 * @return
 */
bool AxonProcessing::processAxons(EdgeCluster* edgeCluster)
{
    if(edgeCluster == nullptr) {
        return false;
    }
    // get necessary values
    OutgoingMessageBuffer* outgoBuffer = edgeCluster->getOutgoingMessageBuffer();

    uint32_t axonId = 0;
    // process axons
    for(KyoChanAxon* axon = edgeCluster->getAxonBlock();
        axon < axon + edgeCluster->getNumberOfAxonBlocks();
        axon++)
    {
        // check border-value to skip some axon
        if(axon->currentState < AXON_PROCESS_BORDER) {
            edgeCluster->getPendingEdges()->checkPendingEdges();
            continue;
        }

        // create new edge
        if((axon->currentState / axon->numberOfEdges) >= AXON_FORK_BORDER) {
            if(rand() % 100 > POSSIBLE_NEXT_LEARNING_STEP) {
                createNewEdge(edgeCluster, axonId, outgoBuffer, m_nextChooser);
            }
        }

        // process normal edges
        KyoChanEdgeSection* edgeSections = edgeCluster->getEdgeBlock();
        for(uint32_t* edgeSectionIds = axon->edgeSections;
            edgeSectionIds < edgeSectionIds + axon->numberOfEdgeSections;
            edgeSectionIds++)
        {
            KyoChanEdgeSection* currentSection = &edgeSections[*edgeSectionIds];
            if(edgeCluster->getClusterType() == EDGE_CLUSTER) {
                processEdgeSection(currentSection, outgoBuffer);
            }
            if(edgeCluster->getClusterType() == NODE_CLUSTER) {
                processEdgeSectionOnNode(currentSection,
                                         outgoBuffer,
                                         ((NodeCluster*)edgeCluster)->getNodeBlock());
            }
        }

        // process pending edges
        if(axon->numberOfPendingEdges != 0) {
            processPendingEdges(axon, outgoBuffer);
        }

        edgeCluster->getPendingEdges()->checkPendingEdges();
        axonId++;
    }
    return true;
}

}
