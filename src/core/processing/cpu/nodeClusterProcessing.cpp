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

#include <messageQueues/incomingMessageBuffer.h>
#include <messageQueues/outgoingMessageBuffer.h>

#include <messages/message.h>
#include <messages/dataMessage.h>

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
    // process normal communication
    std::vector<uint8_t> m_sideOrder = {0, 8};
    for(uint8_t sidePos = 0; sidePos < m_sideOrder.size(); sidePos++)
    {
        const uint8_t side = m_sideOrder[sidePos];

        // get buffer
        Networking::IncomingMessageBuffer* incomBuffer = cluster->getIncomingMessageBuffer(side);
        Networking::OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer(side);

        if(incomBuffer == nullptr) {
            continue;
        }

        uint8_t* data = (uint8_t*)incomBuffer->getMessage()->getPayload();
        uint8_t* end = data + incomBuffer->getMessage()->getPayloadSize();

        while(data < end)
        {
            switch((int)(*data))
            {
                case LEARNING_EDGE_CONTAINER:
                {
                    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;
                    processLerningEdge(cluster, edge->sourceEdgeSectionId, edge->weight, side, outgoBuffer);
                    data += sizeof(KyoChanLearingEdgeContainer);
                    break;
                }
                case PENDING_EDGE_CONTAINER:
                {
                    KyoChanPendingEdgeContainer* edge = (KyoChanPendingEdgeContainer*)data;
                    processPendingEdge(cluster, edge->sourceEdgeSectionId, edge->sourceSide, edge->weight, outgoBuffer);
                    data += sizeof(KyoChanPendingEdgeContainer);
                    break;
                }
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
            edge.targetClusterPath = tempNode.targetClusterPath / 32;
            edge.targetAxonId = tempNode.targetAxonId;
            edge.weight = tempNode.currentState;
            nodeCluster->getOutgoingMessageBuffer(tempNode.targetClusterPath % 32)->addData(&edge);

            numberOfActiveNodes++;
            node->active = 1;
        } else {
            node->active = 0;
        }

        node->currentState /= NODE_COOLDOWN;
        nodeId++;
    }

    return numberOfActiveNodes;
}

/**
 * @brief NodeClusterProcessing::memorizeEdges
 * @param nodeCluster
 * @return
 */
void NodeClusterProcessing::memorizeEdges(NodeCluster *nodeCluster)
{
    Networking::OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer(8);
    KyoChanEdgeSection* start = nodeCluster->getEdgeSectionBlock();
    KyoChanEdgeSection* end = start + nodeCluster->getNumberOfEdgeSections();

    for(KyoChanEdgeSection* section = start;
        section < end;
        section++)
    {
        const float comparismTotalWeight = section->totalWeight;

        // process edge-section
        KyoChanEdge* end = section->edges + section->numberOfEdges;
        for(KyoChanEdge* edge = section->edges;
            edge < end;
            edge++)
        {
            const KyoChanEdge tempEdge = *edge;
            const float diff = tempEdge.weight * (1.0f - tempEdge.memorize);
            //std::cout<<"diff: "<<diff<<"            weight: "<<tempEdge.weight<<std::endl;

            //edge->weight -= diff;
            //section->totalWeight -= diff;
        }
        std::cout<<"comparismTotalWeight: "<<comparismTotalWeight<<"            totalWeight: "<<section->totalWeight<<std::endl;

        // send status upadate to the parent forward-edge-section
        if(comparismTotalWeight - section->totalWeight < 0)
        {
            KyoChanStatusEdgeContainer newEdge;
            newEdge.status = comparismTotalWeight - section->totalWeight;
            newEdge.targetId = section->sourceId;
            //outgoBuffer->addData(&newEdge);
        }
    }
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
 * @brief NodeClusterProcessing::processLerningEdge
 * @param cluster
 * @param sourceEdgeSectionId
 * @param weight
 * @param initSide
 * @param outgoBuffer
 */
inline void NodeClusterProcessing::processLerningEdge(NodeCluster* cluster,
                                                      const uint32_t sourceEdgeSectionId,
                                                      const float weight,
                                                      const uint8_t initSide,
                                                      Networking::OutgoingMessageBuffer* outgoBuffer)
{
    const uint32_t targetEdgeSectionId = cluster->addEmptyEdgeSection(initSide,
                                                                      sourceEdgeSectionId);

    if(targetEdgeSectionId != UNINIT_STATE)
    {
        // create reply-message
        KyoChanLearningEdgeReplyContainer reply;
        reply.sourceEdgeSectionId = sourceEdgeSectionId;
        reply.targetEdgeSectionId = targetEdgeSectionId;

        // send reply-message
        cluster->getOutgoingMessageBuffer(initSide)->addData(&reply);
    }
}

/**
 * @brief NodeClusterProcessing::processPendingEdge
 * @param cluster
 * @param sourceId
 * @param sourceSide
 * @param weight
 * @param outgoBuffer
 */
inline void NodeClusterProcessing::processPendingEdge(NodeCluster *cluster,
                                                      const uint32_t sourceId,
                                                      const uint8_t sourceSide,
                                                      const float weight,
                                                      Networking::OutgoingMessageBuffer *outgoBuffer)
{
    KyoChanEdgeSection* forwardEnd = &((cluster)->getEdgeSectionBlock()[0]);
    const uint32_t numberOfEdgeSections = cluster->getNumberOfEdgeSections();
    KyoChanEdgeSection* forwardStart = &((cluster)->getEdgeSectionBlock()[numberOfEdgeSections - 1]);

    uint32_t forwardEdgeSectionId = numberOfEdgeSections - 1;
    for(KyoChanEdgeSection* forwardEdgeSection = forwardStart;
        forwardEdgeSection >= forwardEnd;
        forwardEdgeSection--)
    {
        if(sourceId == forwardEdgeSection->sourceId
                && sourceSide == forwardEdgeSection->sourceSide)
        {
            processEdgeSection(cluster, forwardEdgeSectionId, weight, outgoBuffer);
        }
        forwardEdgeSectionId--;
    }
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
     // collect necessary values
     const uint16_t numberOfEdge = currentSection->numberOfEdges;
     const uint16_t chooseRange = (numberOfEdge + OVERPROVISIONING) % EDGES_PER_EDGESECTION;
     uint32_t chooseOfExist = rand() % chooseRange;

     if(chooseOfExist >= numberOfEdge)
     {
         // create a new edge-section within the current section
         const uint16_t chooseNewNode = rand() % cluster->getMetaData().numberOfNodes;

         KyoChanEdge newEdge;
         newEdge.targetNodeId =  chooseNewNode;
         currentSection->addEdge(newEdge);

         chooseOfExist = currentSection->numberOfEdges - 1;
     }

     uint8_t splitValue = 5;
     if(partitialWeight <= 1.0f) {
         splitValue = 1;
     }
     for(uint8_t i = 0; i < splitValue; i++)
     {
         KyoChanEdge* tempEdge = &currentSection->edges[rand() % currentSection->numberOfEdges];
         if(cluster->getNodeBlock()[tempEdge->targetNodeId].border
                 <= cluster->getNodeBlock()[tempEdge->targetNodeId].currentState * NODE_COOLDOWN)
         {
             tempEdge->weight -= partitialWeight / (float)splitValue;
         } else {
             tempEdge->weight += partitialWeight / (float)splitValue;
         }
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
inline void NodeClusterProcessing::processEdgeSection(NodeCluster *cluster,
                                                      const uint32_t edgeSectionId,
                                                      const float weight,
                                                      Networking::OutgoingMessageBuffer* outgoBuffer)
{
    assert(cluster->getClusterType() == NODE_CLUSTER);

    if(weight != 0.0f)
    {
        KyoChanEdgeSection* currentSection = &((cluster)->getEdgeSectionBlock()[edgeSectionId]);

        // learning
        if(weight > currentSection->totalWeight) {
            learningEdgeSection(cluster,
                                currentSection,
                                weight - currentSection->totalWeight);
        }

        // process edge-section
        const float ratio = currentSection->totalWeight / weight;
        KyoChanNode* nodes = cluster->getNodeBlock();

        KyoChanEdge* end = currentSection->edges + currentSection->numberOfEdges;
        for(KyoChanEdge* edge = currentSection->edges;
            edge < end;
            edge++)
        {
            const KyoChanEdge tempEdge = *edge;

            // update node with the edge-weight
            nodes[tempEdge.targetNodeId].currentState += tempEdge.weight * ratio;

            // update memorize-value
            const float active = (float)nodes[tempEdge.targetNodeId].active;
            edge->memorize += active * ((1.0f - tempEdge.memorize) / EDGE_MEMORIZE_UPDATE);
        }
    }
}

}
