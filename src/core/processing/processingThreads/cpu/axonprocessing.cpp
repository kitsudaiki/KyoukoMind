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

/**
 * @brief AxonProcessing::AxonProcessing
 * @param nextChooser
 */
AxonProcessing::AxonProcessing(NextChooser *nextChooser)
{
    m_nextChooser = nextChooser;
}

/**
 * @brief processEdgeSection
 * @param currentSection
 * @param outgoBuffer
 */
inline void processEdgeSection(KyoChanEdgeSection* currentSection,
                               OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processEdgeSection")

    uint8_t sideCounter;
    KyoChanEdgeForward* forwardEnd = currentSection->edgeForwards + 16;
    for(KyoChanEdgeForward* forwardEdge = currentSection->edges;
        forwardEdge < forwardEnd;
        forwardEdge++)
    {
        if(forwardEdge->weight != 0.0)
        {
            KyoChanEdgeForewardContainer newEdge;
            newEdge.targetEdgeSectionId = forwardEdge->targetEdgeSectionId;
            newEdge.weight = forwardEdge->weight;

            outgoBuffer->addEdge(sideCounter, &newEdge);
        }
        sideCounter++;
    }
}

/**
 * @brief processEdgeSectionOnNode
 * @param currentSection
 * @param outgoBuffer
 */
inline void processEdgeSectionOnNode(KyoChanEdgeSection* currentSection,
                                     OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processEdgeSection")

    uint8_t sideCounter;
    KyoChanEdgeForward* forwardEnd = currentSection->edgeForwards + 16;
    for(KyoChanEdgeForward* forwardEdge = currentSection->edges;
        forwardEdge < forwardEnd;
        forwardEdge++)
    {
        if(forwardEdge->weight != 0.0)
        {
            KyoChanEdgeForewardContainer newEdge;
            newEdge.targetEdgeSectionId = forwardEdge->targetEdgeSectionId;
            newEdge.weight = forwardEdge->weight;

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
        nodes[edge->targetNodeId].currentState =+ edge->weight;
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
    OUTPUT("---")
    OUTPUT("createNewEdge")
    const uint8_t nextSide = nextChooser->getNextCluster(edgeCluster->getNeighbors(), 14);
    const uint32_t newEdgeId = edgeCluster->getNextNewEdgeId();
    const float weight = 100.0;

    KyoChanLearingEdgeContainer newEdge;
    newEdge.marker = newEdgeId;
    newEdge.sourceAxonId = axonId;
    newEdge.weight = weight;

    std::cout<<"   add learning edge    axonId: "<<axonId<<"   weight: "<<weight<<std::endl;
    outgoBuffer->addLearingEdge(nextSide, &newEdge);

    KyoChanPendingEdgeContainer pendingEdge;
    pendingEdge.marker = newEdgeId;
    pendingEdge.nextSite = nextSide;
    pendingEdge.weight = weight;

    std::cout<<"   add pending edge    newEdgeId: "<<newEdgeId<<"   nextSide: "<<(int)nextSide<<std::endl;
    edgeCluster->getAxonBlock()[axonId].pendingEdges.addPendingEdges(pendingEdge);
}

/**
 * @brief AxonProcessing::processAxons
 * @param edgeCluster
 * @return
 */
bool AxonProcessing::processAxons(EdgeCluster* edgeCluster)
{
    OUTPUT("---")
    OUTPUT("processAxons")
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
        std::cout<<"   axon->currentState: "<<axon->currentState<<std::endl;
        // check border-value to skip some axon
        if(axon->currentState < AXON_PROCESS_BORDER)
        {
            edgeCluster->getPendingEdges()->checkPendingEdges();
            continue;
        }

        // create new edge
        std::cout<<"   axon->currentState: "<<axon->currentState<<std::endl;
        std::cout<<"   axon->numberOfEdges: "<<(axon->numberOfEdges + 1)<<std::endl;
        if((axon->currentState / (axon->numberOfEdges + 1)) >= AXON_FORK_BORDER)
        {
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

        edgeCluster->getPendingEdges()->checkPendingEdges();
        axonId++;
    }
    return true;
}

}
