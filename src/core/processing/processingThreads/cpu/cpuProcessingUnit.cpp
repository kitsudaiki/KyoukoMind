/**
 *  @file    cpuProcessingUnit.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/processing/processingThreads/cpu/cpuProcessingUnit.h>
#include <core/processing/processingThreads/cpu/nextChooser.h>

#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/messaging/messageController.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messages/replyMessage.h>

#include <core/processing/processingThreads/cpu/edgeClusterProcessing.h>
#include <core/processing/processingThreads/cpu/nodeClusterProcessing.h>

namespace KyoukoMind
{

/**
 * @brief CpuProcessingUnit::CpuProcessingUnit
 * @param clusterHandler
 */
CpuProcessingUnit::CpuProcessingUnit(ClusterQueue *clusterQueue):
    ProcessingUnit(clusterQueue)
{
    m_nextChooser = new NextChooser();
    m_edgeClusterProcessing = new EdgeClusterProcessing(m_nextChooser, &m_activeNodes);
    m_nodeClusterProcessing = new NodeClusterProcessing(m_nextChooser, &m_activeNodes);
}

/**
 * @brief CpuProcessingUnit::~CpuProcessingUnit
 */
CpuProcessingUnit::~CpuProcessingUnit()
{
    delete m_edgeClusterProcessing;
    delete m_nextChooser;
}

/**
 * @brief CpuProcessingUnit::processCluster process of a cluster in one cycle
 * @param cluster custer which should be processed
 */
void CpuProcessingUnit::processCluster(Cluster *cluster)
{
    uint8_t clusterType = (uint8_t)cluster->getClusterType();

    uint16_t numberOfActiveNodes = 0;

    // process nodes if cluster is a node-cluster
    if(clusterType == NODE_CLUSTER)
    {
        NodeCluster *nodeCluster = static_cast<NodeCluster*>(cluster);
        numberOfActiveNodes = processNodes(nodeCluster);
        assert(numberOfActiveNodes > 0);
        processMessagesNodeCluster(nodeCluster);
    }
    else
    {
        EdgeCluster *edgeCluster = static_cast<EdgeCluster*>(cluster);
        processMessagesEdgesCluster(edgeCluster);
    }

    // finish the processing-cycle of the current cluster
    cluster->finishCycle(numberOfActiveNodes);
}

/**
 * @brief CpuProcessingUnit::processMessagesNodeCluster
 * @param cluster
 * @return
 */
bool CpuProcessingUnit::processMessagesNodeCluster(NodeCluster *cluster)
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
                    m_nodeClusterProcessing->processEdgeSection(cluster,
                                                                edge->targetEdgeSectionId,
                                                                edge->weight,
                                                                outgoBuffer);
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
 * @brief EdgeProcessing::processIncomingMessages processing of all incoming messages in a cluster
 * @param edgeCluster custer which should be processed
 * @return false if a message-type does not exist, else true
 */
bool CpuProcessingUnit::processMessagesEdgesCluster(EdgeCluster* cluster)
{
    // get buffer
    IncomingMessageBuffer* incomBuffer = cluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer();

    // get number of active nodes from the neighbors
    std::vector<uint8_t> m_sideOrder = {2, 3, 4, 14, 13, 12};
    for(uint8_t sidePos = 0; sidePos < m_sideOrder.size(); sidePos++)
    {
        const uint8_t side = m_sideOrder[sidePos];
        cluster->getNeighbors()[side].activeNodesInNextNodeCluster =
                incomBuffer->getMessage(side)->getMetaData().numberOfActiveNodes;
    }

    // process normal communication
    for(uint8_t sidePos = 0; sidePos < m_sideOrder.size(); sidePos++)
    {
        const uint8_t side = m_sideOrder[sidePos];

        uint8_t* data = (uint8_t*)incomBuffer->getMessage(side)->getPayload();
        uint8_t* end = data + incomBuffer->getMessage(side)->getPayloadSize();

        while(data < end)
        {
            switch((int)(*data))
            {
                case STATUS_EDGE_CONTAINER:
                {
                    KyoChanStatusEdgeContainer* edge = (KyoChanStatusEdgeContainer*)data;
                    m_edgeClusterProcessing->updateEdgeForwardSection(cluster,
                                                                      edge->targetId,
                                                                      edge->status,
                                                                      side,
                                                                      outgoBuffer);
                    data += sizeof(KyoChanStatusEdgeContainer);
                    break;
                }
                case FOREWARD_EDGE_CONTAINER:
                {
                    KyoChanForwardEdgeContainer* edge = (KyoChanForwardEdgeContainer*)data;
                    m_edgeClusterProcessing->processEdgeForwardSection(cluster,
                                                                       edge->targetEdgeSectionId,
                                                                       edge->weight,
                                                                       side,
                                                                       outgoBuffer);
                    data += sizeof(KyoChanForwardEdgeContainer);
                    break;
                }
                case AXON_EDGE_CONTAINER:
                {
                    KyoChanAxonEdgeContainer* edge = (KyoChanAxonEdgeContainer*)data;
                    m_edgeClusterProcessing->processAxon(cluster,
                                                         edge->targetAxonId,
                                                         edge->targetClusterPath,
                                                         edge->weight,
                                                         side,
                                                         outgoBuffer);
                    data += sizeof(KyoChanAxonEdgeContainer);
                    break;
                }
                case LEARNING_EDGE_CONTAINER:
                {
                    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;
                    m_edgeClusterProcessing->processLerningEdge(cluster,
                                                                edge->sourceEdgeSectionId,
                                                                edge->weight,
                                                                side,
                                                                outgoBuffer);
                    data += sizeof(KyoChanLearingEdgeContainer);
                    break;
                }
                case LEARNING_REPLY_EDGE_CONTAINER:
                {
                    KyoChanLearningEdgeReplyContainer* edge = (KyoChanLearningEdgeReplyContainer*)data;

                    KyoChanForwardEdgeSection* edgeForwardSections = cluster->getForwardEdgeSectionBlock();
                    edgeForwardSections[edge->sourceEdgeSectionId].forwardEdges[side].targetId =
                            edge->targetEdgeSectionId;
                    data += sizeof(KyoChanLearningEdgeReplyContainer);
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
 * @brief ClusterProcessing::processNodes processing of the nodes of a specific node-cluster
 * @param nodeCluster node-cluster which should be processed
 * @return number of active nodes in this cluster
 */
uint16_t CpuProcessingUnit::processNodes(NodeCluster* nodeCluster)
{
    assert(nodeCluster != nullptr);

    // get necessary values
    OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer();

    uint16_t nodeId = 0;
    m_activeNodes.numberOfActiveNodes = 0;

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

            // active-node-registration
            if(rand() % 100 <= RANDOM_ADD_ACTIVE_NODE) {
                m_activeNodes.addNodeId(nodeId);
            }
        } else {
            // add randomly a inactive node to the active-node-list to create new connectons
            if(rand() % 100 <= RANDOM_ADD_INACTIVE_NODE) {
                m_activeNodes.addNodeId(nodeId);
            }
        }
        node->currentState /= NODE_COOLDOWN;
        nodeId++;
    }
    // add at least one node to the list of active node even all are inactive
    if(m_activeNodes.numberOfActiveNodes == 0) {
        const uint16_t backupNodeId = rand() % nodeCluster->getNumberOfNodes();
        m_activeNodes.addNodeId(backupNodeId);
    }

    return m_activeNodes.numberOfActiveNodes;
}

}
