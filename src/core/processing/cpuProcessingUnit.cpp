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
#include <core/messaging/messages/replyMessage.h>

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
    KyoChanNode* nodeBlock = nodeCluster->getNodeBlock();
    uint16_t numberOfNodes = nodeCluster->getNumberOfNodes();
    KyoChanAxon* axonBlock = nodeCluster->getAxonBlock();
    uint32_t numberOfAxons = nodeCluster->getNumberOfAxons();

    processIncomingMessages(cluster, axonBlock, numberOfAxons, nodeBlock, numberOfNodes);
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
    KyoChanAxon* axonBlock = edgeCluster->getAxonBlock();
    uint32_t numberOfAxons = edgeCluster->getNumberOfAxons();
    processIncomingMessages(cluster, axonBlock, numberOfAxons);
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
 * @param axonBlock
 * @param numberOfAxons
 * @param nodeBlock
 * @param numberOfNodes
 */
void CpuProcessingUnit::processIncomingMessages(Cluster *cluster,
                                                KyoChanAxon* axonBlock,
                                                const uint16_t numberOfAxons,
                                                KyoChanNode* nodeBlock,
                                                const uint16_t numberOfNodes)
{
    IncomingMessageBuffer* incomBuffer = cluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer();

    // process inputs
    /*if(nodeBlock != nullptr) {
        Message* buffer = incomBuffer->getMessage(0);
        if(buffer != nullptr) {
            for(uint32_t i = 0; i < buffer->getPayloadSize(); i++) {
                processDataMessage(((DataMessage*)(*buffer)[i]),
                                   0,
                                   nodeBlock,
                                   numberOfNodes,
                                   nullptr);
            }
        }
    }

    for(uint8_t side = 0; side < m_sideOrder.size(); side++)
    {
        ClusterID targetId = cluster->getNeighborId(side);
        std::vector<Message*>* buffer = incomBuffer->getMessage(side);
        for(uint32_t j = 0; j < buffer->size(); j++)
        {
            switch((int)buffer->at(j)->getType()) {
            case DATA_MESSAGE:
                processDataMessage((DataMessage*)buffer->at(j),
                                   targetId,
                                   nodeBlock,
                                   numberOfNodes,
                                   outgoBuffer);
                break;
            default:
                break;
            }
        }
    }*/
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
    /*uint8_t numberOfEdges = message->getNumberOfEntries();
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
            outgoBuffer->addEdge(side, &newEdge);
        }
    }*/
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
            KyoChanAxonEdge edge;
            edge.targetClusterPath = nodes->targetClusterPath / 16;
            edge.targetAxonId = nodes->targetAxonId;
            edge.weight = (float)nodes->currentState;
            outgoBuffer->addAxonEdge(side, &edge);
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
                KyoChanMessageEdge newEdge;
                newEdge.weight = edge->weight;
                newEdge.targetNodeId = edge->targetNodeId;
                newEdge.targetClusterPath = edge->targetClusterPath / 16;
                outgoBuffer->addEdge(side, &newEdge);
            }
        }
    }
}

}
