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
        Kitsune::MindMessaging::IncomingMessageBuffer* incomBuffer = cluster->getIncomingMessageBuffer(side);

        if(incomBuffer == nullptr) {
            continue;
        }

        Kitsune::MindMessaging::DataMessage* currentMessage = incomBuffer->getMessage();

        if(currentMessage == nullptr) {
            continue;
        }

        uint8_t* data = currentMessage->getPayload();
        uint8_t* end = data + currentMessage->getPayloadSize();

        while(data < end)
        {
            switch((int)(*data))
            {
                case LEARNING_EDGE_CONTAINER:
                {
                    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;
                    processLerningEdge(cluster, edge->sourceEdgeSectionId, edge->weight, side);
                    data += sizeof(KyoChanLearingEdgeContainer);
                    break;
                }
                case PENDING_EDGE_CONTAINER:
                {
                    KyoChanPendingEdgeContainer* edge = (KyoChanPendingEdgeContainer*)data;
                    processPendingEdge(cluster, edge->sourceEdgeSectionId, edge->sourceSide, edge->weight);
                    data += sizeof(KyoChanPendingEdgeContainer);
                    break;
                }
                case FOREWARD_EDGE_CONTAINER:
                {
                    KyoChanForwardEdgeContainer* edge = (KyoChanForwardEdgeContainer*)data;
                    processEdgeSection(cluster, edge->targetEdgeSectionId, edge->weight);
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
            }
        }
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
            if(node->currentState < 0.0f) {
                node->currentState = 0.0f;
            }
        }
        node->currentState /= NODE_COOLDOWN;
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
    Kitsune::MindMessaging::OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer(8);
    KyoChanEdgeSection* start = nodeCluster->getEdgeSectionBlock();
    KyoChanEdgeSection* end = start + nodeCluster->getNumberOfEdgeSections();

    for(KyoChanEdgeSection* section = start;
        section < end;
        section++)
    {
        const float comparismTotalWeight = section->totalPosWeight;

        // process edge-section
        KyoChanEdge* end = section->edges + section->numberOfEdges;
        for(KyoChanEdge* edge = section->edges;
            edge < end;
            edge++)
        {
            const KyoChanEdge tempEdge = *edge;
            const float diff = tempEdge.weight * (1.0f - tempEdge.memorize);

            edge->weight -= diff;
            section->totalPosWeight -= diff;
        }

        // send status upadate to the parent forward-edge-section
        if(comparismTotalWeight - section->totalPosWeight < 0)
        {
            KyoChanStatusEdgeContainer newEdge;
            newEdge.status = comparismTotalWeight - section->totalPosWeight;
            newEdge.targetId = section->sourceId;
            outgoBuffer->addData(&newEdge);
        }
    }
}

/**
 * @brief NodeClusterProcessing::processLerningEdge
 * @param cluster
 * @param sourceEdgeSectionId
 * @param weight
 * @param initSide
 */
inline void NodeClusterProcessing::processLerningEdge(NodeCluster* cluster,
                                                      const uint32_t sourceEdgeSectionId,
                                                      const float weight,
                                                      const uint8_t initSide)
{
    const uint32_t targetEdgeSectionId = cluster->addEmptyEdgeSection(initSide,
                                                                      sourceEdgeSectionId);

    if(targetEdgeSectionId != UNINIT_STATE_32)
    {
        // create reply-message
        KyoChanLearningEdgeReplyContainer reply;
        reply.sourceEdgeSectionId = sourceEdgeSectionId;
        reply.targetEdgeSectionId = targetEdgeSectionId;
        cluster->getOutgoingMessageBuffer(initSide)->addData(&reply);

        processEdgeSection(cluster, targetEdgeSectionId, weight);
    }
}

/**
 * @brief NodeClusterProcessing::processPendingEdge
 * @param cluster
 * @param sourceId
 * @param sourceSide
 * @param weight
 */
inline void NodeClusterProcessing::processPendingEdge(NodeCluster *cluster,
                                                      const uint32_t sourceId,
                                                      const uint8_t sourceSide,
                                                      const float weight)
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
            processEdgeSection(cluster, forwardEdgeSectionId, weight);
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
     const uint32_t numberOfEdge = currentSection->numberOfEdges;
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

     if(partitialWeight <= 1.0f) {
         return;
     }

     uint8_t numberOfParts = 5;
     for(uint8_t i = 0; i < numberOfParts; i++)
     {
         KyoChanEdge* tempEdge = &currentSection->edges[
                 static_cast<uint32_t>(rand())
                 % currentSection->numberOfEdges];

         const float splitValue = partitialWeight / static_cast<float>(numberOfParts);
         if(cluster->getNodeBlock()[tempEdge->targetNodeId].border
                 <= cluster->getNodeBlock()[tempEdge->targetNodeId].currentState * NODE_COOLDOWN)
         {
             tempEdge->weight -= splitValue;
             currentSection->totalNegWeight -= splitValue;
         }
         else
         {
             tempEdge->weight += splitValue;
             currentSection->totalPosWeight += splitValue;
         }
     }
}

 /**
 * @brief NodeClusterProcessing::processEdgeSection process of a specific edge-section of a cluster
 * @param cluster pointer to the node-custer
 * @param edgeSectionId id of the edge-section within the current cluster
 * @param weight incoming weight-value
 */
inline void NodeClusterProcessing::processEdgeSection(NodeCluster *cluster,
                                                      const uint32_t edgeSectionId,
                                                      const float weight)
{
    assert(cluster->getClusterType() == NODE_CLUSTER);

    if(weight != 0.0f)
    {
        KyoChanEdgeSection* currentSection = &((cluster)->getEdgeSectionBlock()[edgeSectionId]);
        const float totalWeight = currentSection->totalPosWeight + ((-1)*currentSection->totalNegWeight);

        // learning
        if(weight - totalWeight > 1.0f) {
            learningEdgeSection(cluster, currentSection, weight - totalWeight);
        }

        // process edge-section
        float ratio = totalWeight / weight;
        if(ratio > 1.0f) {
            ratio = 1.0f;
        }
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
            const float active = static_cast<float>(nodes[tempEdge.targetNodeId].active);
            edge->memorize += active * ((1.0f - tempEdge.memorize) / EDGE_MEMORIZE_UPDATE);
        }
    }
}

}
