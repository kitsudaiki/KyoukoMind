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

#include <core/processing/processingThreads/cpu/clusterProcessing.h>
#include <core/processing/processingThreads/cpu/messageProcessing.h>

namespace KyoukoMind
{

/**
 * @brief CpuProcessingUnit::CpuProcessingUnit
 * @param clusterHandler
 */
CpuProcessingUnit::CpuProcessingUnit(ClusterQueue *clusterQueue):
    ProcessingUnit(clusterQueue)
{
    m_sideOrder = {0, 2, 3, 4, 8, 14, 13, 12};
    m_nextChooser = new NextChooser();
    m_clusterProcessing = new ClusterProcessing(m_nextChooser, &m_activeNodes);
    m_messageProcessing = new MessageProcessing(m_clusterProcessing);
}

/**
 * @brief CpuProcessingUnit::~CpuProcessingUnit
 */
CpuProcessingUnit::~CpuProcessingUnit()
{
    delete m_messageProcessing;
    delete m_clusterProcessing;
    delete m_nextChooser;
}

/**
 * @brief CpuProcessingUnit::processCluster process of a cluster in one cycle
 * @param cluster custer which should be processed
 */
void CpuProcessingUnit::processCluster(EdgeCluster *cluster)
{
    uint8_t clusterType = (uint8_t)cluster->getClusterType();

    uint16_t numberOfActiveNodes = 0;

    // process nodes if cluster is a node-cluster
    if(clusterType == NODE_CLUSTER) {
        NodeCluster *nodeCluster = static_cast<NodeCluster*>(cluster);
        numberOfActiveNodes = processNodes(nodeCluster);
    }

    // process messages of the cluster
    processMessagesEdges(cluster);

    // finish the processing-cycle of the current cluster
    cluster->finishCycle(numberOfActiveNodes);
}

/**
 * @brief EdgeProcessing::processIncomingMessages processing of all incoming messages in a cluster
 * @param edgeCluster custer which should be processed
 * @return false if a message-type does not exist, else true
 */
bool CpuProcessingUnit::processMessagesEdges(EdgeCluster* cluster)
{
    // get buffer
    IncomingMessageBuffer* incomBuffer = cluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer();

    // get number of active nodes from the neighbors
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
                    m_messageProcessing->processStatusEdge(data, side, cluster, outgoBuffer);
                    data += sizeof(KyoChanStatusEdgeContainer);
                    break;

                case INTERNAL_EDGE_CONTAINER:
                    m_messageProcessing->processInternalEdge(data, cluster, outgoBuffer);
                    data += sizeof(KyoChanInternalEdgeContainer);
                    break;

                case DIRECT_EDGE_CONTAINER:
                    m_messageProcessing->processDirectEdge(data, cluster);
                    data += sizeof(KyoChanDirectEdgeContainer);
                    break;

                case FOREWARD_EDGE_CONTAINER:
                    m_messageProcessing->processForwardEdge(data, side, cluster, outgoBuffer);
                    data += sizeof(KyoChanForwardEdgeContainer);
                    break;

                case AXON_EDGE_CONTAINER:
                    m_messageProcessing->processAxonEdge(data, side, cluster, outgoBuffer);
                    data += sizeof(KyoChanAxonEdgeContainer);
                    break;

                case LEARNING_EDGE_CONTAINER:
                    m_messageProcessing->processLerningEdge(data, side, cluster, outgoBuffer);
                    data += sizeof(KyoChanLearingEdgeContainer);
                    break;

                case LEARNING_REPLY_EDGE_CONTAINER:
                    m_messageProcessing->processLearningReply(data, side, cluster);
                    data += sizeof(KyoChanLearningEdgeReplyContainer);
                    break;

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
    //std::cout<<"---"<<std::endl;
    //std::cout<<"processNodes"<<std::endl;
    //std::cout<<"    cluster-id: "<<nodeCluster->getClusterId()<<std::endl;
    // precheck
    if(nodeCluster->getClusterType() == EDGE_CLUSTER) {
        return 0;
    }

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
            std::cout<<"---"<<std::endl;
            std::cout<<"processNodes"<<std::endl;
            std::cout<<"    cluster-id: "<<nodeCluster->getClusterId()<<std::endl;
            std::cout<<"        active node: "<<(int)nodeId<<std::endl;
            std::cout<<"        tempNode.currentState: "<<tempNode.currentState<<std::endl;
            // send message
            const uint8_t side = tempNode.targetClusterPath % 17;
            if(side == 0)
            {
                m_clusterProcessing->processEdgeForwardSection(nodeCluster,
                                                               tempNode.targetAxonId,
                                                               tempNode.currentState,
                                                               8,
                                                               outgoBuffer);
            }
            else
            {
                // create new axon-edge
                KyoChanAxonEdgeContainer edge;
                edge.targetClusterPath = tempNode.targetClusterPath / 17;
                edge.targetAxonId = tempNode.targetAxonId;
                edge.weight = tempNode.currentState;
                outgoBuffer->addAxonEdge(side, &edge);
            }

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
