/**
 *  @file    cpuProcessingUnit.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/processing/cpuProcessingUnit.h>
#include <core/processing/nextChooser.h>

#include <core/cluster/cluster.h>
#include <core/cluster/emptyCluster.h>
#include <core/cluster/edgeCluster.h>
#include <core/cluster/nodeCluster.h>

#include <core/messaging/messageController.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messages/dataAxonMessage.h>
#include <core/messaging/messages/replyMessage.h>
#include <core/messaging/messages/learningMessage.h>
#include <core/messaging/messages/learningReplyMessage.h>
#include <core/messaging/messages/cycleFinishMessage.h>

namespace KyoukoMind
{

/**
 * @brief CpuProcessingUnit::CpuProcessingUnit
 * @param clusterHandler
 */
CpuProcessingUnit::CpuProcessingUnit(ClusterQueue *clusterQueue):
    ProcessingUnit(clusterQueue)
{
    m_sideOrder = {2,3,4,13,12,11};
    m_nextChooser = new NextChooser();
}

/**
 * @brief CpuProcessingUnit::~CpuProcessingUnit
 */
CpuProcessingUnit::~CpuProcessingUnit()
{

}

/**
 * @brief CpuProcessingUnit::processCluster
 * @param cluster
 */
void CpuProcessingUnit::processCluster(Cluster *cluster)
{
    switch((int)cluster->getClusterType())
    {
        case EMPTY_CLUSTER:
            processEmptyCluster(cluster);
            break;
        case EDGE_CLUSTER:
            processEdgeCluster(cluster);
            processEmptyCluster(cluster);
            break;
        case NODE_CLUSTER:
            processNodeCluster(cluster);
            processEmptyCluster(cluster);
            break;
        default:
            break;
    }
}

/**
 * @brief CpuProcessingUnit::processNodeCluster
 * @param cluster
 * @return
 */
bool CpuProcessingUnit::processNodeCluster(Cluster *cluster)
{
    NodeCluster* nodeCluster = static_cast<NodeCluster*>(cluster);
    if(nodeCluster == nullptr) {
        return false;
    }
    KyoChanNode* nodeBlock =  nodeCluster->getNodeBlock();
    uint16_t numberOfNodes = nodeCluster->getNumberOfNodes();
    processIncomingMessages(cluster, nodeBlock, numberOfNodes);
    OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer();
    processNodes(nodeCluster, outgoBuffer);
    return true;
}

/**
 * @brief CpuProcessingUnit::processEdgeCluster
 * @param cluster
 * @return
 */
bool CpuProcessingUnit::processEdgeCluster(Cluster *cluster)
{
    EdgeCluster* edgeCluster = static_cast<EdgeCluster*>(cluster);
    if(edgeCluster == nullptr) {
        return false;
    }
    processIncomingMessages(cluster, nullptr, 0);
    OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer();
    processAxons(edgeCluster, outgoBuffer);
    return true;
}

/**
 * @brief CpuProcessingUnit::processEmptyCluster
 * @param cluster
 * @return
 */
bool CpuProcessingUnit::processEmptyCluster(Cluster *cluster)
{
    EmptyCluster* emptyCluster = static_cast<EmptyCluster*>(cluster);
    if(emptyCluster == nullptr) {
        return false;
    }
    emptyCluster->finishCycle();
    return true;
}

/**
 * @brief CpuProcessingUnit::processIncomingMessages
 * @param cluster
 * @param nodeBlock
 * @param numberOfNodes
 */
void CpuProcessingUnit::processIncomingMessages(Cluster *cluster,
                                                KyoChanNode* nodeBlock,
                                                const uint16_t numberOfNodes)
{
    IncomingMessageBuffer* incomBuffer = cluster->getIncomingMessageBuffer();

    // process inputs
    if(nodeBlock != nullptr) {
        std::vector<Message*>* buffer = incomBuffer->getMessageQueue(0);
        if(buffer != nullptr) {
            for(uint32_t i = 0; i < buffer->size(); i++) {
                processDataMessage(((DataMessage*)(*buffer)[i]),
                                   0,
                                   nodeBlock,
                                   numberOfNodes,
                                   nullptr);
            }
        }
    }

    for(uint8_t i = 0; i < m_sideOrder.size(); i++)
    {
        ClusterID targetId = cluster->getNeighborId(i);
        std::vector<Message*>* buffer = incomBuffer->getMessageQueue(i);

    }
}

/**
 * @brief CpuProcessingUnit::processDataMessage
 * @param message
 * @param targetId
 * @param nodeBlock
 * @param numberOfNodes
 * @param outgoBuffer
 */
void CpuProcessingUnit::processDataMessage(DataMessage* message,
                                           const ClusterID targetId,
                                           KyoChanNode* nodeBlock,
                                           const uint16_t numberOfNodes,
                                           OutgoingMessageBuffer* outgoBuffer)
{
    uint8_t numberOfEdges = message->getNumberOfEdges();
    for(KyoChanEdge* edge = message->getEdges();
        edge < edge + numberOfEdges;
        edge++)
    {
        if(edge->targetClusterPath == 0)
        {
            if(numberOfNodes > edge->targetNodeId) {
                nodeBlock[edge->targetNodeId].currentState += edge->weight;
            }
        } else {
            uint8_t side = edge->targetClusterPath % 16;
            KyoChanEdge newEdge = *edge;
            newEdge.targetClusterPath /= 16;
            outgoBuffer->addEdge(targetId, 15-side, newEdge);
        }
    }
}

/**
 * @brief CpuProcessingUnit::processDataAxonMessage
 * @param message
 * @param targetId
 * @param axonBlock
 * @param numberOfAxons
 * @param outgoBuffer
 */
void CpuProcessingUnit::processDataAxonMessage(DataAxonMessage *message,
                                               const ClusterID targetId,
                                               KyoChanAxon *axonBlock,
                                               const uint16_t numberOfAxons,
                                               OutgoingMessageBuffer *outgoBuffer)
{
    uint8_t numberOfAxonEdges = message->getNumberOfAxonEdges();
    for(KyoChanAxonEdge* axonEdge = message->getAxonEdges();
        axonEdge < axonEdge + numberOfAxonEdges;
        axonEdge++)
    {
        if(axonEdge->targetClusterPath == 0)
        {
            if(numberOfAxons > axonEdge->targetAxonId) {
                axonBlock[axonEdge->targetAxonId].currentState += axonEdge->weight;
            }
        } else {
            uint8_t side = axonEdge->targetClusterPath % 16;
            KyoChanAxonEdge newAxonEdge = *axonEdge;
            newAxonEdge.targetClusterPath /= 16;
            outgoBuffer->addAxonEdge(targetId, 15-side, newAxonEdge);
        }
    }
}

/**
 * @brief CpuProcessingUnit::processLearningMessage
 * @param message
 * @param nodeBlock
 * @param numberOfNodes
 */
void CpuProcessingUnit::processLearningMessage(LearningMessage *message,
                                               const ClusterID targetId,
                                               const uint8_t inputSide,
                                               KyoChanNode *nodeBlock,
                                               const uint16_t numberOfNodes,
                                               OutgoingMessageBuffer *outgoBuffer)
{
    uint8_t numberOfEdges = message->getNumberOfNewEdges();
    for(KyoChanNewEdge* newEdges = message->getNewEdges();
        newEdges < newEdges + numberOfEdges;
        newEdges++)
    {
        bool goToNext = false;

        if(nodeBlock != nullptr) {
            if(rand() % 100 <= POSSIBLE_NEXT_AXON_STEP) {
                 goToNext = true;
            }
        } else {

        }
    }
}

/**
 * @brief CpuProcessingUnit::processLearningRyplyMessage
 * @param message
 * @param nodeBlock
 * @param numberOfNodes
 */
void CpuProcessingUnit::processLearningRyplyMessage(LearningReplyMessage *message,
                                                    const ClusterID targetId,
                                                    const uint8_t inputSide,
                                                    KyoChanNode *nodeBlock,
                                                    const uint16_t numberOfNodes,
                                                    OutgoingMessageBuffer *outgoBuffer)
{

}

/**
 * @brief CpuProcessingUnit::processNodes
 * @param nodeCluster
 * @param outgoBuffer
 */
void CpuProcessingUnit::processNodes(NodeCluster *nodeCluster,
                                     OutgoingMessageBuffer *outgoBuffer)
{
    uint8_t numberOfNodes = nodeCluster->getNumberOfNodes();
    for(KyoChanNode* nodes = nodeCluster->getNodeBlock();
        nodes < nodes + numberOfNodes;
        nodes++)
    {
        if(nodes->border <= nodes->currentState) {
            uint8_t side = nodes->targetClusterPath % 16;
            ClusterID neighborId = nodeCluster->getNeighborId(side);
            KyoChanEdge edge;
            edge.targetClusterPath = nodes->targetClusterPath / 16;
            edge.targetNodeId = nodes->targetAxonId;
            edge.weight = (float)nodes->currentState;
            outgoBuffer->addEdge(neighborId, 15-side, edge);
        }
        nodes->currentState /= NODE_COOLDOWN;
    }
}

/**
 * @brief CpuProcessingUnit::processAxons
 * @param edgeCluster
 * @param outgoBuffer
 */
void CpuProcessingUnit::processAxons(EdgeCluster *edgeCluster,
                                     OutgoingMessageBuffer *outgoBuffer)
{
    KyoChanEdgeSection* edgeSections = edgeCluster->getEdgeBlock();
    uint8_t numberOfAxons = edgeCluster->getNumberOfAxonBlocks();
    for(KyoChanAxon* axons = edgeCluster->getAxonBlock();
        axons < axons + numberOfAxons;
        axons++)
    {
        for(uint32_t* edgeSectionIds = axons->edgeSections;
            edgeSectionIds < edgeSectionIds + axons->numberOfEdgeSections;
            edgeSectionIds++)
        {
            KyoChanEdgeSection* currentSection = &edgeSections[*edgeSectionIds];
            for(KyoChanEdge* edge = currentSection->edges;
                edge < edge + currentSection->numberOfEdges;
                edge++)
            {
                uint8_t side = edge->targetClusterPath % 16;
                ClusterID neighborId = edgeCluster->getNeighborId(side);
                KyoChanEdge newEdge = *edge;
                newEdge.targetClusterPath /= 16;
                outgoBuffer->addEdge(neighborId, 15-side, newEdge);
            }
        }
    }
}

}
