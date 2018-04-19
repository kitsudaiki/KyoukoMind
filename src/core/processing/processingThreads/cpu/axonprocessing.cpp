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

#include <core/clustering/cluster/cluster.h>
#include <core/clustering/cluster/emptyCluster.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

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
    KyoChanEdge* end = currentSection->edges + currentSection->numberOfEdges;
    for(KyoChanEdge* edge = currentSection->edges;
        edge < end;
        edge++)
    {
        uint8_t side = edge->targetClusterPath % 16;

        KyoChanEdgeContainer newEdge;
        newEdge.weight = edge->weight;
        newEdge.targetNodeId = edge->targetNodeId;
        newEdge.targetClusterPath = edge->targetClusterPath / 16;

        outgoBuffer->addEdge(side, &newEdge);
    }
}

inline void processEdgeSectionOnNode(KyoChanEdgeSection* currentSection,
                                     OutgoingMessageBuffer* outgoBuffer,
                                     KyoChanNode* nodes)
{
    // process edge-section
    KyoChanEdge* end = currentSection->edges + currentSection->numberOfEdges;
    for(KyoChanEdge* edge = currentSection->edges;
        edge < end;
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
    const uint8_t nextSide = nextChooser->getNextCluster(edgeCluster->getNeighbors(), 14);
    const uint32_t newEdgeId = edgeCluster->getNextNewEdgeId();
    const float weight = 1.0;

    KyoChanLearingEdgeContainer newEdge;
    newEdge.newEdgeId = newEdgeId;
    newEdge.sourceAxonId = axonId;
    newEdge.weight = weight;

    outgoBuffer->addLearingEdge(nextSide, &newEdge);

    KyoChanPendingEdgeContainer pendingEdge;
    pendingEdge.newEdgeId = newEdgeId;
    pendingEdge.nextSite = nextSide;
    pendingEdge.weight = weight;

    edgeCluster->getAxonBlock()[axonId].pendingEdges.addPendingEdges(pendingEdge);
}

/**
 * @brief AxonProcessing::processPendingEdges
 * @param axon
 * @param outgoBuffer
 */
inline void processPendingEdges(KyoChanAxon* axon,
                                OutgoingMessageBuffer* outgoBuffer)
{
    KyoChanPendingEdgeContainer* end = axon->pendingEdges.pendingEdges + MAX_PENDING_EDGES;
    for(KyoChanPendingEdgeContainer* pendingEdge = axon->pendingEdges.pendingEdges;
        pendingEdge < end;
        pendingEdge++)
    {
        if(pendingEdge->newEdgeId != 0)
        {
            uint8_t side = pendingEdge->nextSite;

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
    KyoChanAxon* axonEnd = edgeCluster->getAxonBlock() + edgeCluster->getNumberOfAxonBlocks();
    for(KyoChanAxon* axon = edgeCluster->getAxonBlock();
        axon < axonEnd;
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
        uint32_t* edgeSectionIdEnd = axon->edgeSections + axon->numberOfEdgeSections;
        for(uint32_t* edgeSectionIds = axon->edgeSections;
            edgeSectionIds < edgeSectionIdEnd;
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
