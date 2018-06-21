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

namespace KyoukoMind
{

/**
 * @brief NodeClusterProcessing::NodeClusterProcessing
 */
NodeClusterProcessing::NodeClusterProcessing()
{
}


/**
 * @brief NodeClusterProcessing::processMessagesNodeCluster
 * @param cluster
 * @return
 */
bool NodeClusterProcessing::processMessagesNodeCluster(NodeCluster *cluster)
{
    // get buffer
    IncomingMessageBuffer* incomBuffer = cluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer();

    // process normal communication
    std::vector<uint8_t> m_sideOrder = {0, 8};
    for(uint8_t sidePos = 0; sidePos < m_sideOrder.size(); sidePos++)
    {
        const uint8_t side = m_sideOrder[sidePos];

        uint8_t* data = (uint8_t*)incomBuffer->getMessage(side)->getPayload();
        uint8_t* end = data + incomBuffer->getMessage(side)->getPayloadSize();

        while(data < end)
        {
            switch((int)(*data))
            {
                case FOREWARD_EDGE_CONTAINER:
                {
                    KyoChanForwardEdgeContainer* edge = (KyoChanForwardEdgeContainer*)data;
                    processEdgeSection(cluster, edge->targetEdgeSectionId, edge->weight, outgoBuffer);
                    data += sizeof(KyoChanForwardEdgeContainer);
                    break;
                }
                case DIRECT_EDGE_CONTAINER:
                {
                    KyoChanDirectEdgeContainer* edge = (KyoChanDirectEdgeContainer*)data;
                    cluster->getNodeBlock()[edge->targetNodeId].currentState += edge->weight;
                    data += sizeof(KyoChanDirectEdgeContainer);
                    break;
                }
                default:
                    return false;
                    break;
            }
        }
        //incomBuffer->getMessage(side)->closeBuffer();
        //delete incomBuffer->getMessage(side);
    }

    return true;
}


/**
 * @brief NodeClusterProcessing::processNodes processing of the nodes of a specific node-cluster
 * @param nodeCluster node-cluster which should be processed
 * @return number of active nodes in this cluster
 */
uint16_t NodeClusterProcessing::processNodes(NodeCluster* nodeCluster)
{
    assert(nodeCluster != nullptr);

    // get necessary values
    OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer();

    uint16_t nodeId = 0;
    uint16_t numberOfActiveNodes = 0;

    // process nodes
    KyoChanNode* start = nodeCluster->getNodeBlock();
    KyoChanNode* end = start + nodeCluster->getNumberOfNodes();

    for(KyoChanNode* node = start;
        node < end;
        node++)
    {
        const KyoChanNode tempNode = *node;

        if(tempNode.border <= tempNode.currentState)
        {
            // create new axon-edge
            KyoChanAxonEdgeContainer edge;
            edge.targetClusterPath = tempNode.targetClusterPath / 17;
            edge.targetAxonId = tempNode.targetAxonId;
            edge.weight = tempNode.currentState;
            outgoBuffer->addAxonEdge(tempNode.targetClusterPath % 17, &edge);

            numberOfActiveNodes++;
        }

        node->currentState /= NODE_COOLDOWN;
        nodeId++;
    }

    return numberOfActiveNodes;
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
  * @param cluster pointer to the node-custer
  * @param currentSection edge-section with should learn the new value
  * @param partitialWeight weight-difference to learn
  */
 inline void NodeClusterProcessing::learningEdgeSection(NodeCluster *cluster,
                                                        KyoChanEdgeSection* currentSection,
                                                        const float partitialWeight)
 {
     std::cout<<"---"<<std::endl;
     std::cout<<"learningEdgeSection"<<std::endl;

     // collect necessary values
     const uint16_t numberOfEdge = currentSection->numberOfEdges;
     const uint16_t chooseRange = (numberOfEdge + OVERPROVISIONING) % EDGES_PER_EDGESECTION;
     uint16_t chooseOfExist = rand() % chooseRange;

     if(chooseOfExist >= numberOfEdge)
     {
         // create a new edge-section within the current section
         const uint16_t chooseNewNode = rand() % cluster->getMetaData().numberOfNodes;

         KyoChanEdge newEdge;
         newEdge.targetNodeId =  chooseNewNode;
         currentSection->addEdge(newEdge);

         chooseOfExist = currentSection->numberOfEdges - 1;
     }

     // update a existing edge
     currentSection->updateWeight(chooseOfExist,
                                  partitialWeight / 2.0);

     // update two other already existing edges
     if(numberOfEdge > 0)
     {
         currentSection->updateWeight(rand() % numberOfEdge,
                                      partitialWeight / 4.0);
         currentSection->updateWeight(rand() % numberOfEdge,
                                      partitialWeight / 4.0);
     } }

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
        const float comparismTotalWeight = currentSection->totalWeight;

        // learning
        if(weight > currentSection->totalWeight) {
            learningEdgeSection(cluster,
                                currentSection,
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

        // send status upadate to the parent forward-edge-section
        KyoChanStatusEdgeContainer newEdge;
        newEdge.status = comparismTotalWeight - currentSection->totalWeight;
        newEdge.targetId = currentSection->sourceId;
        outgoBuffer->addStatusEdge(8, &newEdge);
    }
}

}
