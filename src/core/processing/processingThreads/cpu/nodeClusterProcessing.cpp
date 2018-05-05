/**
 *  @file    nodeClusterProcessing.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "nodeClusterProcessing.h"

#include <core/clustering/cluster/cluster.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/processing/processingThreads/cpu/nextChooser.h>

#include <core/messaging/messageController.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messages/replyMessage.h>

#include <core/processing/processingThreads/cpu/processingMethods.h>

namespace KyoukoMind
{

NodeClusterProcessing::NodeClusterProcessing(NextChooser* nextChooser)
{
    m_sideOrder = {2,3,4,13,12,11};
    m_nextChooser = nextChooser;
}

/**
 * @brief processIncomDirectEdge
 * @param data
 * @param nodeCluster
 * @param outgoBuffer
 */
inline void processIncomDirectEdge(uint8_t *data,
                                   NodeCluster* nodeCluster)
{
    OUTPUT("---")
    OUTPUT("processIncomDirectEdge")
    KyoChanDirectEdgeContainer* edge = (KyoChanDirectEdgeContainer*)data;

    std::cout<<"    weight: "<<edge->weight<<"    edge->targetNodeId: "<<(int)edge->targetNodeId<<std::endl;
    nodeCluster->getNodeBlock()[edge->targetNodeId].currentState += edge->weight;
}

/**
 * @brief processIncomForwardEdge
 * @param data
 * @param nodeCluster
 * @param outgoBuffer
 */
inline void processForwardEdge(uint8_t *data,
                                    NodeCluster* nodeCluster,
                                    OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processIncomForwardEdge")
    KyoChanEdgeForwardContainer* edge = (KyoChanEdgeForwardContainer*)data;

    std::cout<<"    weight: "<<edge->weight<<"    clusterID: "<<nodeCluster->getClusterId()<<std::endl;
    processEdgeSection(&nodeCluster->getEdgeBlock()[edge->targetEdgeSectionId],
                       edge->weight,
                       nodeCluster->getNodeBlock(),
                       outgoBuffer);
}

/**
 * @brief processIncomLearningReply
 * @param data
 * @param initSide
 * @param cluster
 */
inline void processLearningReply(uint8_t *data,
                                      uint8_t initSide,
                                      NodeCluster* cluster)
{
    OUTPUT("---")
    OUTPUT("processIncomLearningReply")
    KyoChanLearningEdgeReplyContainer* edge = (KyoChanLearningEdgeReplyContainer*)data;

    KyoChanEdgeSection* edgeSections = cluster->getEdgeBlock();
    edgeSections[edge->sourceEdgeSectionId].forwardEdges[initSide].targetEdgeSectionId =
            edge->targetEdgeSectionId;
}

/**
 * @brief NodeClusterProcessing::processInputMessages
 * @param nodeCluster
 * @return
 */
bool NodeClusterProcessing::processDirectMessages(NodeCluster* nodeCluster)
{
    OUTPUT("---")
    OUTPUT("processInputMessages")
    IncomingMessageBuffer* incomBuffer = nodeCluster->getIncomingMessageBuffer();

    uint8_t* start = (uint8_t*)incomBuffer->getMessage(0)->getPayload();
    uint8_t* end = start + incomBuffer->getMessage(0)->getPayloadSize();
    for(uint8_t* data = start;
        data < end;
        data += sizeof(KyoChanDirectEdgeContainer))
    {
        processIncomDirectEdge(data, nodeCluster);
    }
    //incomBuffer->getMessage(0)->closeBuffer();
    //delete incomBuffer->getMessage(0);
    return true;
}

/**
 * @brief NodeClusterProcessing::processAxons
 * @param cluster
 * @return
 */
bool NodeClusterProcessing::processAxons(NodeCluster* cluster)
{
    OUTPUT("---")
    OUTPUT("processAxons")
    if(cluster == nullptr) {
        return false;
    }
    // get necessary values
    OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer();

    // process axons
    KyoChanAxon* axonEnd = cluster->getAxonBlock() + cluster->getNumberOfAxonBlocks();
    for(KyoChanAxon* axon = cluster->getAxonBlock();
        axon < axonEnd;
        axon++)
    {
        std::cout<<"    axon->currentState: "<<axon->currentState<<std::endl;
        if(axon->currentState < AXON_PROCESS_BORDER) {
            continue;
        }

        // process normal edges
        KyoChanEdgeSection* edgeSections = cluster->getEdgeBlock();
        processEdgeSection(&edgeSections[axon->edgeSectionId],
                           axon->currentState,
                           cluster->getNodeBlock(),
                           outgoBuffer);
    }
    return true;
}

/**
 * @brief NodeClusterProcessing::processNodes
 * @param nodeCluster
 * @return
 */
bool NodeClusterProcessing::processNodes(NodeCluster* nodeCluster)
{
    OUTPUT("---")
    OUTPUT("processNodes")
    if(nodeCluster == nullptr) {
        return false;
    }
    // get necessary values
    OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer();
    const uint16_t numberOfNodes = nodeCluster->getNumberOfNodes();
    KyoChanAxon* axonBlock = nodeCluster->getAxonBlock();

    // process nodes
    KyoChanNode* end = nodeCluster->getNodeBlock() + numberOfNodes;
    for(KyoChanNode* nodes = nodeCluster->getNodeBlock();
        nodes < end;
        nodes++)
    {
        std::cout<<"    nodes->currentState: "<<nodes->currentState<<std::endl;
        if(nodes->border <= nodes->currentState)
        {
            if(nodes->targetClusterPath != 0)
            {
                const uint8_t side = nodes->targetClusterPath % 16;
                // create new axon-edge
                KyoChanAxonEdgeContainer edge;
                edge.targetClusterPath = nodes->targetClusterPath / 16;
                edge.targetAxonId = nodes->targetAxonId;
                edge.weight = nodes->currentState;

                outgoBuffer->addAxonEdge(side, &edge);
            }
            else
            {
                axonBlock[nodes->targetAxonId].currentState = nodes->currentState;
            }

        }
        nodes->currentState /= NODE_COOLDOWN;
    }
    return true;
}

/**
 * @brief NodeClusterProcessing::processIncomingMessages
 * @param edgeCluster
 * @return
 */
bool NodeClusterProcessing::processMessagesEdges(NodeCluster* nodeCluster)
{
    OUTPUT("---")
    OUTPUT("processIncomingMessages")
    if(nodeCluster == nullptr) {
        return false;
    }

    IncomingMessageBuffer* incomBuffer = nodeCluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer();

    // process normal communication
    for(uint8_t sidePos = 0; sidePos < m_sideOrder.size(); sidePos++)
    {
        const uint8_t side = m_sideOrder[sidePos];
        uint8_t* start = (uint8_t*)incomBuffer->getMessage(side)->getPayload();
        uint8_t* end = start + incomBuffer->getMessage(side)->getPayloadSize();

        uint8_t* data = start;
        while(data < end)
        {
            switch((int)(*data))
            {
                case DIRECT_EDGE_CONTAINER:
                    data += sizeof(KyoChanDirectEdgeContainer);
                    break;
                case FOREWARD_EDGE_CONTAINER:
                    processForwardEdge(data, nodeCluster, outgoBuffer);
                    data += sizeof(KyoChanEdgeForwardContainer);
                    break;
                case AXON_EDGE_CONTAINER:
                    processAxonEdge(data, nodeCluster->getAxonBlock(), outgoBuffer);
                    data += sizeof(KyoChanAxonEdgeContainer);
                    break;
                case PENDING_EDGE_CONTAINER:
                    processPendingEdge(data, nodeCluster, outgoBuffer);
                    data += sizeof(KyoChanPendingEdgeContainer);
                    break;
                case LEARNING_EDGE_CONTAINER:
                    processLerningEdge(data, side, nodeCluster, outgoBuffer);
                    data += sizeof(KyoChanLearingEdgeContainer);
                    break;
                case LEARNING_REPLY_EDGE_CONTAINER:
                    processLearningReply(data, side, nodeCluster);
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

}
