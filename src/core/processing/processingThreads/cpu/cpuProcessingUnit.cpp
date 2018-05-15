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
    m_sideOrder = {0, 2, 3, 4, 13, 12, 11};
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
 * @brief CpuProcessingUnit::processCluster
 * @param cluster
 */
void CpuProcessingUnit::processCluster(EdgeCluster *cluster)
{
    //OUTPUT("processCluster")
    uint8_t clusterType = (uint8_t)cluster->getClusterType();

    uint16_t numberOfActiveNodes = 0;
    if(clusterType == NODE_CLUSTER)
    {
        NodeCluster *nodeCluster = static_cast<NodeCluster*>(cluster);
        numberOfActiveNodes = processNodes(nodeCluster);
    }
    processMessagesEdges(cluster);

    cluster->finishCycle(numberOfActiveNodes);
}

/**
 * @brief EdgeProcessing::processIncomingMessages
 * @param edgeCluster
 * @return
 */
bool CpuProcessingUnit::processMessagesEdges(EdgeCluster* cluster)
{
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
                case INTERNAL_EDGE_CONTAINER:
                    m_messageProcessing->processInternalEdge(data, cluster);
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

            //incomBuffer->getMessage(side)->closeBuffer();
            //delete incomBuffer->getMessage(side);
        }
    }

    return true;
}

/**
 * @brief ClusterProcessing::processNodes
 * @param nodeCluster
 * @return
 */
uint16_t CpuProcessingUnit::processNodes(NodeCluster* nodeCluster)
{
    OUTPUT("---")
    OUTPUT("processNodes")
    uint16_t numberOfActiveNodes = 0;

    if(nodeCluster == nullptr) {
        return 0;
    }

    // get necessary values
    OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer();

    uint16_t nodeId = 0;
    m_activeNodes.numberOfActiveNodes = 0;

    // process nodes
    KyoChanNode* end = nodeCluster->getNodeBlock() + nodeCluster->getNumberOfNodes();
    for(KyoChanNode* node = nodeCluster->getNodeBlock();
        node < end;
        node++)
    {
        std::cout<<"    nodes->currentState: "<<node->currentState<<std::endl;
        if(node->border <= node->currentState)
        {
            // create new axon-edge
            KyoChanAxonEdgeContainer edge;
            edge.targetClusterPath = node->targetClusterPath >> 4;
            edge.targetAxonId = node->targetAxonId;
            edge.weight = node->currentState;

            // send message
            const uint8_t side = node->targetClusterPath % 16;
            outgoBuffer->addAxonEdge(side, &edge);

            // active-node-registration
            if(rand() % 100 <= RANDOM_ADD_ACTIVE_NODE) {
                m_activeNodes.addNodeId(nodeId);
            }
            numberOfActiveNodes++;
        } else {
            if(rand() % 100 <= RANDOM_ADD_INACTIVE_NODE) {
                m_activeNodes.addNodeId(nodeId);
            }
        }
        node->currentState /= NODE_COOLDOWN;
        nodeId++;
    }
    if(numberOfActiveNodes == 0) {
        const uint16_t backupNodeId = rand() % nodeCluster->getNumberOfNodes();
        m_activeNodes.addNodeId(backupNodeId);
    }
    return numberOfActiveNodes;
}

}
