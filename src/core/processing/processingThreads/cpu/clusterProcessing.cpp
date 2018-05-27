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
    std::cout<<"---"<<std::endl;
    std::cout<<"updateEdgeForwardSection"<<std::endl;
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
    std::cout<<"---"<<std::endl;
    std::cout<<"learningForwardEdgeSection"<<std::endl;
    std::cout<<"    cluster-id: "<<cluster->getClusterId()<<std::endl;
    std::cout<<"    partitialWeight: "<<partitialWeight<<std::endl;
    std::cout<<"    inititalSide: "<<(int)inititalSide<<std::endl;
    // get side of learning-process
    const uint8_t nextSide = m_nextChooser->getNextCluster(cluster->getNeighbors(),
                                                           inititalSide,
                                                           cluster->getClusterType());
    std::cout<<"    nextSide: "<<(int)nextSide<<std::endl;

    // cluster-internal learning
    if(currentSection->forwardEdges[nextSide].targetId == UNINIT_STATE
            && nextSide == 8)
    {
        NodeCluster* nodeCluster = static_cast<NodeCluster*>(cluster);
        const uint32_t newSectionId = nodeCluster->addEmptyEdgeSection();
        currentSection->forwardEdges[nextSide].targetId = newSectionId;
    }

    // cluster-external lerning
    if(currentSection->forwardEdges[nextSide].targetId == UNINIT_STATE
            && nextSide != 8
            && nextSide != 16)
    {
        // send new learning-edge
        KyoChanLearingEdgeContainer newEdge;
        newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
        newEdge.weight = partitialWeight;
        outgoBuffer->addLearingEdge(nextSide, &newEdge);

        currentSection->setPedingBit(nextSide);
    }

    if(currentSection->forwardEdges[nextSide].targetId == UNINIT_STATE
            && nextSide == 16)
    {
        // send new learning-edge
        KyoChanDirectEdgeContainer newEdge;
        newEdge.targetNodeId = 0;
        newEdge.weight = partitialWeight;
        outgoBuffer->addDirectEdge(nextSide, &newEdge);

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
    std::cout<<"---"<<std::endl;
    std::cout<<"processEdgeForwardSection"<<std::endl;
    std::cout<<"    cluster-id: "<<cluster->getClusterId()<<std::endl;
    KyoChanForwardEdgeSection* currentSection = &((cluster)->getForwardEdgeSectionBlock()[forwardEdgeSectionId]);
    std::cout<<"    totalWeight: "<<currentSection->totalWeight<<std::endl;

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

        const float ratio = currentSection->totalWeight / weight;
        // normal processing
        uint8_t sideCounter = 2; // to skip side number 0 and 1
        KyoChanForwardEdge* forwardStart = currentSection->forwardEdges + sideCounter;
        KyoChanForwardEdge* forwardEnd = currentSection->forwardEdges + 17;

        for(KyoChanForwardEdge* forwardEdge = forwardStart;
            forwardEdge < forwardEnd;
            forwardEdge++)
        {
            const KyoChanForwardEdge tempForwardEdge = *forwardEdge;

            if(forwardEdge->weight != 0.0)
            {
                // internal edge
                if(sideCounter == 8)
                {
                    /*KyoChanInternalEdgeContainer newEdge;
                    newEdge.targetEdgeSectionId = tempForwardEdge.targetId;
                    newEdge.weight = tempForwardEdge.weight * ratio;
                    outgoBuffer->addInternalEdge(sideCounter, &newEdge);*/
                    processEdgeSection((NodeCluster*)cluster,
                                       tempForwardEdge.targetId,
                                       tempForwardEdge.weight * ratio,
                                       outgoBuffer);
                }

                // normal external edge
                if(sideCounter <= 14 && sideCounter != 8)
                {
                    KyoChanForwardEdgeContainer newEdge;
                    newEdge.targetEdgeSectionId = tempForwardEdge.targetId;
                    newEdge.weight = tempForwardEdge.weight * ratio;
                    outgoBuffer->addForwardEdge(sideCounter, &newEdge);
                }

                // outgoing edge
                if(sideCounter == 16)
                {
                    KyoChanDirectEdgeContainer newEdge;
                    newEdge.targetNodeId = (int16_t)tempForwardEdge.targetId;
                    newEdge.weight = tempForwardEdge.weight * ratio;
                    outgoBuffer->addDirectEdge(sideCounter, &newEdge);
                }
            }

            //std::cout<<"    memorize: "<<forwardEdge->memorize<<std::endl;
            const float diff = tempForwardEdge.weight * (1.0f - tempForwardEdge.memorize);
            forwardEdge->weight *= tempForwardEdge.memorize;
            currentSection->totalWeight -= diff;

            currentSection->zeroPendingBit(sideCounter);
            sideCounter++;
        }
    }
}

 /**
  * @brief ClusterProcessing::learningEdgeSection learing-process of the specific edge-section
  * @param currentSection edge-section with should learn the new value
  * @param partitialWeight weight-difference to learn
  */
 inline void ClusterProcessing::learningEdgeSection(KyoChanEdgeSection* currentSection,
                                                    const float partitialWeight)
 {
     std::cout<<"---"<<std::endl;
     std::cout<<"learningEdgeSection"<<std::endl;
     std::cout<<"    partitialWeight: "<<partitialWeight<<std::endl;
     // collect necessary values
     const uint16_t numberOfEdge = currentSection->numberOfEdges;
     const uint16_t chooseRange = (numberOfEdge + OVERPROVISIONING) % EDGES_PER_EDGESECTION;
     const uint16_t chooseOfExist = rand() % chooseRange;
     std::cout<<"    chooseOfExist: "<<(int)chooseOfExist<<std::endl;

     if(chooseOfExist >= numberOfEdge)
     {
         // create a new edge-section within the current section
         const uint16_t chooseNewNode = rand() % m_activeNodes->numberOfActiveNodes;

         KyoChanEdge newEdge;
         newEdge.targetNodeId =  m_activeNodes->nodeIds[chooseNewNode];
         newEdge.weight = partitialWeight / 2.0;
         currentSection->addEdge(newEdge);
     }
     else
     {
         // update a existing edge
         currentSection->edges[chooseOfExist].weight += partitialWeight / 2.0;
     }

     // update two other already existing edges
     if(numberOfEdge > 0) {
         currentSection->edges[rand() % numberOfEdge].weight += partitialWeight / 4.0;
         currentSection->edges[rand() % numberOfEdge].weight += partitialWeight / 4.0;
     }
     currentSection->totalWeight += partitialWeight;
 }

 /**
 * @brief ClusterProcessing::processEdgeSection process of a specific edge-section of a cluster
 * @param cluster pointer to the node-custer
 * @param edgeSectionId id of the edge-section within the current cluster
 * @param weight incoming weight-value
 * @param outgoBuffer pointer to outgoing message-buffer
 */
void ClusterProcessing::processEdgeSection(NodeCluster *cluster,
                                           uint32_t edgeSectionId,
                                           const float weight,
                                           OutgoingMessageBuffer* outgoBuffer)
{
    assert(cluster->getClusterType() == NODE_CLUSTER);
    std::cout<<"---"<<std::endl;
    std::cout<<"processEdgeSection"<<std::endl;
    if(weight != 0.0)
    {
        KyoChanEdgeSection* currentSection = &((cluster)->getEdgeSectionBlock()[edgeSectionId]);

        // learning
        if(weight > currentSection->totalWeight) {
            learningEdgeSection(currentSection,
                                weight - currentSection->totalWeight);
        }

        // process edge-section
        const float ratio = currentSection->totalWeight / weight;
        KyoChanNode* nodes = cluster->getNodeBlock();

        // float updateValue = 0;
        KyoChanEdge* end = currentSection->edges + currentSection->numberOfEdges;
        for(KyoChanEdge* edge = currentSection->edges;
            edge < end;
            edge++)
        {
            const KyoChanEdge tempEdge = *edge;
            // update node with the edge-weight
            nodes[tempEdge.targetNodeId].currentState += tempEdge.weight * ratio;

            // update memorize-value
            if(nodes[tempEdge.targetNodeId].border
                    <= nodes[tempEdge.targetNodeId].currentState * NODE_COOLDOWN)
            {
                edge->memorize += (1.0f - tempEdge.memorize) / EDGE_MEMORIZE_UPDATE;
            } else {
                edge->memorize -= (1.0f - tempEdge.memorize) / EDGE_MEMORIZE_UPDATE;
            }

            // memorize the current edge-weight
            const float diff = tempEdge.weight * (1.0f - tempEdge.memorize);
            edge->weight *= tempEdge.memorize;
            currentSection->totalWeight -= diff;
        }

        //KyoChanStatusEdgeContainer newEdge;
        //newEdge.status = updateValue;
        //newEdge.targetId = currentSection->sourceId;
        //outgoBuffer->addStatusEdge(8, &newEdge);
    }
}

}
