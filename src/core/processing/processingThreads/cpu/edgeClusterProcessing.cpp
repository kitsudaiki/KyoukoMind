/**
 *  @file    edgeClusterProcessing.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "edgeClusterProcessing.h"

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

EdgeClusterProcessing::EdgeClusterProcessing(NextChooser* nextChooser)
{
    m_sideOrder = {2,3,4,13,12,11};
    m_nextChooser = nextChooser;
}

/**
 * @brief processIncomForwardEdge
 * @param data
 * @param nodeCluster
 * @param outgoBuffer
 */
inline void processIncomForwardEdge(uint8_t *data,
                                    EdgeCluster* edgeCluster,
                                    OutgoingMessageBuffer* outgoBuffer)
{
    KyoChanEdgeForwardContainer* edge = (KyoChanEdgeForwardContainer*)data;

    processEdgeForwardSection(&edgeCluster->getEdgeBlock()[edge->targetEdgeSectionId],
                              edge->weight,
                              outgoBuffer);
}

/**
 * @brief processIncomLearningReply
 * @param data
 * @param initSide
 * @param cluster
 */
inline void processIncomLearningReply(uint8_t *data,
                                      uint8_t initSide,
                                      EdgeCluster* cluster)
{
    KyoChanLearningEdgeReplyContainer* edge = (KyoChanLearningEdgeReplyContainer*)data;

    KyoChanEdgeForwardSection* edgeForwardSections = cluster->getEdgeBlock();
    edgeForwardSections[edge->sourceEdgeSectionId].edgeForwards[initSide].targetEdgeSectionId =
            edge->targetEdgeSectionId;
}

/**
 * @brief EdgeProcessing::processInputMessages
 * @param nodeCluster
 * @return
 */
bool EdgeClusterProcessing::processInputMessages(EdgeCluster* cluster)
{

    IncomingMessageBuffer* incomBuffer = cluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer();

    uint8_t* start = (uint8_t*)incomBuffer->getMessage(0)->getPayload();
    uint8_t* end = start + incomBuffer->getMessage(0)->getPayloadSize();
    for(uint8_t* data = start;
        data < end;
        data += data[1])
    {
        processIncomForwardEdge(data, cluster, outgoBuffer);
    }
    //incomBuffer->getMessage(0)->closeBuffer();
    //delete incomBuffer->getMessage(0);
    return true;
}

/**
 * @brief AxonProcessing::processAxons
 * @param cluster
 * @return
 */
bool EdgeClusterProcessing::processAxons(EdgeCluster* cluster)
{
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
        if(axon->currentState < AXON_PROCESS_BORDER) {
            continue;
        }

        // process normal edges
        KyoChanEdgeForwardSection* edgeForwardSections = cluster->getEdgeBlock();
        processEdgeForwardSection(&edgeForwardSections[axon->edgeSectionId],
                                  axon->currentState,
                                  outgoBuffer);
    }
    return true;
}

/**
 * @brief EdgeProcessing::processIncomingMessages
 * @param edgeCluster
 * @return
 */
bool EdgeClusterProcessing::processIncomingMessages(EdgeCluster* cluster)
{
    if(cluster == nullptr) {
        return false;
    }

    IncomingMessageBuffer* incomBuffer = cluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer();

    // process normal communication
    for(uint8_t sidePos = 0; sidePos < m_sideOrder.size(); sidePos++)
    {
        uint8_t side = m_sideOrder[sidePos];
        uint8_t* start = (uint8_t*)incomBuffer->getMessage(side)->getPayload();
        uint8_t* end = start + incomBuffer->getMessage(side)->getPayloadSize();

        uint8_t* data = start;
        while(data < end)
        {
            std::cout<<"data[1]: "<<(int)data[1]<<std::endl;
            switch((int)(*data))
            {
                case DIRECT_EDGE_CONTAINER:
                    data += sizeof(KyoChanDirectEdgeContainer);
                    break;
                case EDGE_FOREWARD_CONTAINER:
                    processIncomForwardEdge(data, cluster, outgoBuffer);
                    data += sizeof(KyoChanEdgeForwardContainer);
                    break;
                case AXON_EDGE_CONTAINER:
                    processIncomAxonEdge(data, cluster->getAxonBlock(), outgoBuffer);
                    data += sizeof(KyoChanAxonEdgeContainer);
                    break;
                case LEARNING_CONTAINER:
                    processIncomLerningEdge(data, side, cluster, outgoBuffer);
                    data += sizeof(KyoChanLearingEdgeContainer);
                    break;
                case LEARNING_REPLY_CONTAINER:
                    processIncomLearningReply(data, side, cluster);
                    data += sizeof(KyoChanLearningEdgeReplyContainer);
                    break;
                default:
                    data = end;
                    break;
            }

            //incomBuffer->getMessage(side)->closeBuffer();
            //delete incomBuffer->getMessage(side);
        }
    }

    return true;
}

}
