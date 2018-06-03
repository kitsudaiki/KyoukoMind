/**
 *  @file    clusterProcessing.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "nodeClusterProcessing.h"

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
 * @brief NodeClusterProcessing::ClusterProcessing
 * @param nextChooser
 */
NodeClusterProcessing::NodeClusterProcessing(NextChooser* nextChooser,
                                             PossibleKyoChanNodes* activeNodes)
{
    m_nextChooser = nextChooser;
    m_activeNodes = activeNodes;
}

/**
 * @brief NodeClusterProcessing::randFloat
 * @param b
 * @return
 */
inline float NodeClusterProcessing::randFloat(const float b)
{
    const float random = ((float) rand()) / (float) UNINIT_STATE;
    return random * b;
}

 /**
  * @brief NodeClusterProcessing::learningEdgeSection learing-process of the specific edge-section
  * @param currentSection edge-section with should learn the new value
  * @param partitialWeight weight-difference to learn
  */
 inline void NodeClusterProcessing::learningEdgeSection(KyoChanEdgeSection* currentSection,
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
 * @brief NodeClusterProcessing::processEdgeSection process of a specific edge-section of a cluster
 * @param cluster pointer to the node-custer
 * @param edgeSectionId id of the edge-section within the current cluster
 * @param weight incoming weight-value
 * @param outgoBuffer pointer to outgoing message-buffer
 */
void NodeClusterProcessing::processEdgeSection(NodeCluster *cluster,
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
