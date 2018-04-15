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
            m_axonBlock = ((EdgeCluster*)m_currentCluster)->getAxonBlock();
            m_numberOfAxons = ((EdgeCluster*)m_currentCluster)->getNumberOfAxons();
            processIncomingMessages();
            processAxons();
            checkPendingClusterEdges();
            ((EmptyCluster*)m_currentCluster)->finishCycle();
            break;
        case NODE_CLUSTER:
            m_axonBlock = ((EdgeCluster*)m_currentCluster)->getAxonBlock();
            m_numberOfAxons = ((EdgeCluster*)m_currentCluster)->getNumberOfAxons();
            m_nodeBlock = ((NodeCluster*)m_currentCluster)->getNodeBlock();
            m_numberOfNodes = ((NodeCluster*)m_currentCluster)->getNumberOfNodes();
            processIncomingMessages();
            processNodes();
            processAxons();
            checkPendingClusterEdges();
            ((EmptyCluster*)m_currentCluster)->finishCycle();
            break;
        default:
            break;
    }

    m_currentClusterType = UNDEFINED_CLUSTER;
    m_currentCluster = nullptr;
    m_axonBlock = nullptr;
    m_numberOfAxons = 0;
    m_nodeBlock = nullptr;
    m_numberOfNodes = 0;
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
    if(m_currentClusterType == NODE_CLUSTER)
    {
        for(uint8_t* data = (uint8_t*)incomBuffer->getMessage(0)->getPayload();
            data < data + incomBuffer->getMessage(0)->getPayloadSize();
            data++)
        {
            processIncomEdge(data, outgoBuffer);
        }
    }

    // process normal communication
    for(uint8_t side = 0; side < m_sideOrder.size(); side++)
    {
        for(uint8_t* data = (uint8_t*)incomBuffer->getMessage(side)->getPayload();
            data < data + incomBuffer->getMessage(side)->getPayloadSize();
            data++)
        {
            switch((int)(*data))
            {
                case EDGE_CONTAINER:
                    processIncomEdge(data, outgoBuffer);
                    break;
                case PENDING_EDGE_CONTAINER:
                    processIncomPendingEdge(data, outgoBuffer);
                    break;
                case AXON_EDGE_CONTAINER:
                    processIncomAxonEdge(data, outgoBuffer);
                    break;
                case LEARNING_CONTAINER:
                    processIncomLerningEdge(data, side, outgoBuffer);
                    break;
                case LEARNING_REPLY_CONTAINER:
                    processIncomLerningReplyEdge(data, side, outgoBuffer);
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
void CpuProcessingUnit::processIncomEdge(uint8_t *data,
                                         OutgoingMessageBuffer* outgoBuffer)
{
    KyoChanMessageEdge* edge = (KyoChanMessageEdge*)data;
    if(edge->targetClusterPath != 0)
    {
        uint8_t side = edge->targetClusterPath % 16;
        edge->targetClusterPath /= 16;
        outgoBuffer->addEdge(side, edge);
    }
    else
    {
        m_nodeBlock[edge->targetNodeId].currentState += edge->weight;
    }
}

/**
 * @brief CpuProcessingUnit::processIncomPendingEdge
 * @param data
 * @param outgoBuffer
 */
void CpuProcessingUnit::processIncomPendingEdge(uint8_t *data,
                                                OutgoingMessageBuffer *outgoBuffer)
{
    KyoChanPendingEdge* edge = (KyoChanPendingEdge*)data;
    EdgeCluster* edgeCluster = static_cast<EdgeCluster*>(m_currentCluster);

    for(KyoChanPendingEdge* pendingEdge = edgeCluster->getPendingEdges();
        pendingEdge < pendingEdge + edgeCluster->m_numberOfPendingEdges;
        pendingEdge++)
    {
        if(pendingEdge->newEdgeId == edge->newEdgeId)
        {
            outgoBuffer->addPendingEdge(pendingEdge->nextSite, edge);
            return;
        }
    }
}

/**
 * @brief CpuProcessingUnit::processIncomAxonEdge
 * @param data
 * @return
 */
void CpuProcessingUnit::processIncomAxonEdge(uint8_t *data,
                                             OutgoingMessageBuffer* outgoBuffer)
{
    KyoChanAxonEdge* edge = (KyoChanAxonEdge*)data;
    if(edge->targetClusterPath != 0)
    {
        uint8_t side = edge->targetClusterPath % 16;
        edge->targetClusterPath /= 16;
        outgoBuffer->addAxonEdge(side, edge);
    }
    else
    {
        m_axonBlock[edge->targetAxonId].currentState = edge->weight;
    }
}

/**
 * @brief CpuProcessingUnit::processIncomLerningEdge
 * @param data
 * @return
 */
void CpuProcessingUnit::processIncomLerningEdge(uint8_t *data,
                                                uint8_t initSide,
                                                OutgoingMessageBuffer* outgoBuffer)
{
    KyoChanNewEdge* edge = (KyoChanNewEdge*)data;
    EdgeCluster* edgeCluster = static_cast<EdgeCluster*>(m_currentCluster);

    // check abort-condition
    if(edge->step == 8 && m_currentClusterType != NODE_CLUSTER) {
        return;
    }

    if(m_currentClusterType != NODE_CLUSTER
            || rand() % 100 > POSSIBLE_NEXT_LEARNING_STEP)
    {
        uint8_t nextSide = m_nextChooser->getNextCluster(m_currentCluster->getNeighbors(), initSide);
        edge->sourceClusterPath = (edge->sourceClusterPath << 16) + initSide;
        edge->step++;
        outgoBuffer->addLearingEdge(nextSide, edge);

        KyoChanPendingEdge pendEdge;
        pendEdge.newEdgeId = edge->newEdgeId;
        pendEdge.nextSite = nextSide;

        addPendingEdges(pendEdge,
                        edgeCluster->getPendingEdges(),
                        edgeCluster->getNumberOfMaxPendingEdges());
    }
    else
    {
        uint16_t nodeId = rand() % m_numberOfNodes;
        m_nodeBlock[nodeId].currentState += edge->weight;

        KyoChanNewEdgeReply reply;
        reply.newEdgeId = edge->newEdgeId;
        reply.sourceAxonId = edge->sourceAxonId;
        reply.sourceClusterPath = edge->sourceClusterPath;
        reply.targetNodeId = nodeId;

        outgoBuffer->addLearningReplyMessage(initSide, &reply);
    }
}

/**
 * @brief CpuProcessingUnit::processIncomLerningReplyEdge
 * @param data
 * @return
 */
void CpuProcessingUnit::processIncomLerningReplyEdge(uint8_t *data,
                                                     uint8_t initSide,
                                                     OutgoingMessageBuffer* outgoBuffer)
{
    KyoChanNewEdgeReply* edge = (KyoChanNewEdgeReply*)data;
    if(edge->sourceClusterPath != 0)
    {
        edge->targetClusterPath = (edge->targetClusterPath << 16) + initSide;
        uint8_t side = edge->sourceClusterPath % 16;
        edge->sourceClusterPath /= 16;
        outgoBuffer->addLearningReplyMessage(side, edge);
    }
    else
    {
        KyoChanAxon* axon = &(((EdgeCluster*)m_currentCluster)->getAxonBlock()[edge->sourceAxonId]);
        for(KyoChanPendingEdge* pendingEdge = &(axon->pendingEdges[0]);
            pendingEdge < pendingEdge + MAX_PENDING_EDGES;
            pendingEdge++)
        {
            if(pendingEdge->newEdgeId == edge->newEdgeId)
            {
                pendingEdge->newEdgeId = 0;
                axon->numberOfPendingEdges--;

                KyoChanEdge newEdge;
                newEdge.weight = pendingEdge->weight;
                newEdge.targetClusterPath = edge->targetClusterPath;
                newEdge.targetNodeId = edge->targetNodeId;

                ((EdgeCluster*)m_currentCluster)->addEdge(edge->sourceAxonId, newEdge);
                return;
            }
        }
    }
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

    // get necessary values
    OutgoingMessageBuffer* outgoBuffer = m_currentCluster->getOutgoingMessageBuffer();
    NodeCluster* nodeCluster = static_cast<NodeCluster*>(m_currentCluster);
    uint8_t numberOfNodes = nodeCluster->getNumberOfNodes();

    // process nodes
    for(KyoChanNode* nodes = nodeCluster->getNodeBlock();
        nodes < nodes + numberOfNodes;
        nodes++)
    {
        if(nodes->border <= nodes->currentState)
        {
            if(nodes->targetClusterPath != 0) {
                uint8_t side = nodes->targetClusterPath % 16;

                KyoChanAxonEdge edge;
                edge.targetClusterPath = nodes->targetClusterPath / 16;
                edge.targetAxonId = nodes->targetAxonId;
                edge.weight = (float)nodes->currentState;

                outgoBuffer->addAxonEdge(side, &edge);
            }
            else
            {
                m_axonBlock[nodes->targetAxonId].currentState = nodes->currentState;
            }

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

    // get necessary values
    OutgoingMessageBuffer* outgoBuffer = m_currentCluster->getOutgoingMessageBuffer();
    EdgeCluster* edgeCluster = static_cast<EdgeCluster*>(m_currentCluster);
    KyoChanEdgeSection* edgeSections = edgeCluster->getEdgeBlock();
    uint8_t numberOfAxons = edgeCluster->getNumberOfAxonBlocks();

    // process axons
    for(KyoChanAxon* axons = edgeCluster->getAxonBlock();
        axons < axons + numberOfAxons;
        axons++)
    {
        // check border-value to skip some axon
        if(axons->currentState < AXON_PROCESS_BORDER) {
            axons->numberOfPendingEdges -= checkPendingEdges(&axons->pendingEdges[0],
                                                             MAX_PENDING_EDGES);
            continue;
        }
        float multi = axons->currentState / (float)axons->numberOfEdges;

        // process normal edges
        for(uint32_t* edgeSectionIds = axons->edgeSections;
            edgeSectionIds < edgeSectionIds + axons->numberOfEdgeSections;
            edgeSectionIds++)
        {
            KyoChanEdgeSection* currentSection = &edgeSections[*edgeSectionIds];

            // process edge-section
            for(KyoChanEdge* edge = currentSection->edges;
                edge < edge + currentSection->numberOfEdges;
                edge++)
            {
                uint8_t side = edge->targetClusterPath % 16;

                KyoChanMessageEdge newEdge;
                newEdge.weight = multi * edge->weight;
                newEdge.targetNodeId = edge->targetNodeId;
                newEdge.targetClusterPath = edge->targetClusterPath / 16;

                outgoBuffer->addEdge(side, &newEdge);
            }
        }

        // process pending edges
        if(axons->numberOfPendingEdges != 0)
        {
            for(KyoChanPendingEdge* pendingEdge = &axons->pendingEdges[0];
                pendingEdge < pendingEdge + MAX_PENDING_EDGES;
                pendingEdge++)
            {
                if(pendingEdge->newEdgeId != 0)
                {
                    uint8_t side = pendingEdge->targetClusterPath % 16;

                    KyoChanMessageEdge newEdge;
                    newEdge.weight = multi * pendingEdge->weight;

                    outgoBuffer->addEdge(side, &newEdge);
                }
            }
        }

        axons->numberOfPendingEdges -= checkPendingEdges(&axons->pendingEdges[0],
                                                         MAX_PENDING_EDGES);
    }
    return true;
}

/**
 * @brief CpuProcessingUnit::checkPendingClusterEdges
 */
void CpuProcessingUnit::checkPendingClusterEdges()
{
    EdgeCluster* edgeCluster = static_cast<EdgeCluster*>(m_currentCluster);
    edgeCluster->m_numberOfPendingEdges -= checkPendingEdges(edgeCluster->getPendingEdges(),
                                                             edgeCluster->getNumberOfMaxPendingEdges());
}

/**
 * @brief CpuProcessingUnit::checkPendingEdges
 * @param pendingEdges
 * @param numberOfPendingEdges
 * @return
 */
uint8_t CpuProcessingUnit::checkPendingEdges(KyoChanPendingEdge *pendingEdges,
                                             const uint8_t numberOfPendingEdges)
{
    uint8_t counter = 0;
    for(KyoChanPendingEdge* pendingEdge = pendingEdges;
        pendingEdge < pendingEdge + numberOfPendingEdges;
        pendingEdge++)
    {
        pendingEdge->validCounter++;
        if(pendingEdge->validCounter >= MAX_PENDING_VALID_CYCLES) {
            KyoChanPendingEdge emptyEdge;
            *pendingEdge = emptyEdge;
            counter++;
        }
    }
    return counter;
}

/**
 * @brief CpuProcessingUnit::addPendingEdges
 * @param newEdge
 * @param pendingEdges
 * @param numberOfPendingEdges
 * @return
 */
bool CpuProcessingUnit::addPendingEdges(const KyoChanPendingEdge &newEdge,
                                        KyoChanPendingEdge *pendingEdges,
                                        const uint8_t numberOfPendingEdges)
{
    for(KyoChanPendingEdge* pendingEdge = pendingEdges;
        pendingEdge < pendingEdge + numberOfPendingEdges;
        pendingEdge++)
    {
        if(pendingEdge->newEdgeId == 0) {
            *pendingEdge = newEdge;
            return true;
        }
    }
    return false;
}

}
