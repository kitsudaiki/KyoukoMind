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
    m_currentClusterType = (uint8_t)cluster->getClusterType();
    m_currentCluster = cluster;

    switch((int)m_currentClusterType)
    {
        case EMPTY_CLUSTER:
            ((EmptyCluster*)m_currentCluster)->finishCycle();
            break;
        case EDGE_CLUSTER:
            processIncomingMessages();
            processAxons();
            ((EmptyCluster*)m_currentCluster)->finishCycle();
            break;
        case NODE_CLUSTER:
            processIncomingMessages();
            processNodes();
            processAxons();
            ((EmptyCluster*)m_currentCluster)->finishCycle();
            break;
        default:
            break;
    }

    m_currentClusterType = UNDEFINED_CLUSTER;
    m_currentCluster = nullptr;
}

/**
 * @brief CpuProcessingUnit::processIncomingMessages
 */
bool CpuProcessingUnit::processIncomingMessages()
{
    if(m_currentCluster == nullptr) {
        return false;
    }
    IncomingMessageBuffer* incomBuffer = m_currentCluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = m_currentCluster->getOutgoingMessageBuffer();

    // process inputs
    if(m_currentClusterType == NODE_CLUSTER) {
        for(uint8_t* data = (uint8_t*)incomBuffer->getMessage(0)->getPayload();
            data < data + incomBuffer->getMessage(0)->getPayloadSize();
            data++)
        {
            processIncomEdge(data);
        }
    }

    for(uint8_t side = 0; side < m_sideOrder.size(); side++)
    {
        for(uint8_t* data = (uint8_t*)incomBuffer->getMessage(side)->getPayload();
            data < data + incomBuffer->getMessage(side)->getPayloadSize();
            data++)
        {
            switch((int)(*data))
            {
                case EDGE_CONTAINER:
                    processIncomEdge(data);
                    break;
                case AXON_EDGE_CONTAINER:
                    processIncomAxonEdge(data);
                    break;
                case LEARNING_CONTAINER:
                    processIncomLerningEdge(data);
                    break;
                case LEARNING_REPLY_CONTAINER:
                    processIncomLerningReplyEdge(data);
                    break;
                default:
                    break;
            }
        }
    }

    return true;
}

/**
 * @brief CpuProcessingUnit::processIncomEdge
 * @param data
 * @return
 */
bool CpuProcessingUnit::processIncomEdge(uint8_t *data)
{

}

/**
 * @brief CpuProcessingUnit::processIncomAxonEdge
 * @param data
 * @return
 */
bool CpuProcessingUnit::processIncomAxonEdge(uint8_t *data)
{

}

/**
 * @brief CpuProcessingUnit::processIncomLerningEdge
 * @param data
 * @return
 */
bool CpuProcessingUnit::processIncomLerningEdge(uint8_t *data)
{

}

/**
 * @brief CpuProcessingUnit::processIncomLerningReplyEdge
 * @param data
 * @return
 */
bool CpuProcessingUnit::processIncomLerningReplyEdge(uint8_t *data)
{

}

/**
 * @brief CpuProcessingUnit::processNodes
 * @return
 */
bool CpuProcessingUnit::processNodes()
{
    if(m_currentClusterType == NODE_CLUSTER) {
        return false;
    }
    OutgoingMessageBuffer* outgoBuffer = m_currentCluster->getOutgoingMessageBuffer();
    NodeCluster* nodeCluster = static_cast<NodeCluster*>(m_currentCluster);
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
    return true;
}

/**
 * @brief CpuProcessingUnit::processAxons
 * @return
 */
bool CpuProcessingUnit::processAxons()
{
    if(m_currentClusterType == NODE_CLUSTER
            || m_currentClusterType == EDGE_CLUSTER) {
        return false;
    }
    OutgoingMessageBuffer* outgoBuffer = m_currentCluster->getOutgoingMessageBuffer();
    EdgeCluster* edgeCluster = static_cast<EdgeCluster*>(m_currentCluster);
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
    return true;
}

}
