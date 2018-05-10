/**
 *  @file    clusterProcessing.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "clusterProcessing.h"

namespace KyoukoMind
{

/**
 * @brief ClusterProcessing::ClusterProcessing
 * @param nextChooser
 */
ClusterProcessing::ClusterProcessing(NextChooser* nextChooser)
{
    m_sideOrder = {0, 2, 3, 4, 13, 12, 11};
    m_nextChooser = nextChooser;
}

/**
 * @brief EdgeProcessing::processIncomingMessages
 * @param edgeCluster
 * @return
 */
bool ClusterProcessing::processMessagesEdges(EdgeCluster* cluster)
{
    IncomingMessageBuffer* incomBuffer = cluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer();

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
                case DIRECT_EDGE_CONTAINER:
                    processDirectEdge(data, cluster);
                    data += sizeof(KyoChanDirectEdgeContainer);
                    break;
                case FOREWARD_EDGE_CONTAINER:
                    processForwardEdge(data, cluster, outgoBuffer);
                    data += sizeof(KyoChanEdgeForwardContainer);
                    break;
                case AXON_EDGE_CONTAINER:
                    processAxonEdge(data, outgoBuffer);
                    data += sizeof(KyoChanAxonEdgeContainer);
                    break;
                case PENDING_EDGE_CONTAINER:
                    processPendingEdge(data, cluster, outgoBuffer);
                    data += sizeof(KyoChanPendingEdgeContainer);
                    break;
                case LEARNING_EDGE_CONTAINER:
                    processLerningEdge(data, side, cluster, outgoBuffer);
                    data += sizeof(KyoChanLearingEdgeContainer);
                    break;
                case LEARNING_REPLY_EDGE_CONTAINER:
                    processLearningReply(data, side, cluster);
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
bool ClusterProcessing::processNodes(NodeCluster* nodeCluster)
{
    OUTPUT("---")
    OUTPUT("processNodes")
    if(nodeCluster == nullptr) {
        return false;
    }
    // get necessary values
    OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer();
    const uint16_t numberOfNodes = nodeCluster->getNumberOfNodes();
    KyoChanForwardEdgeSection* forwardBlock = nodeCluster->getForwardEdgeSectionBlock();

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
                processEdgeForwardSection(&forwardBlock[nodes->targetAxonId],
                                          nodes->currentState,
                                          outgoBuffer);
            }

        }
        nodes->currentState /= NODE_COOLDOWN;
    }
    return true;
}

/**
 * @brief processEdgeForwardSection
 * @param currentSection
 * @param weight
 * @param outgoBuffer
 */
 void ClusterProcessing::processEdgeForwardSection(KyoChanForwardEdgeSection* currentSection,
                                                       const float weight,
                                                       OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processEdgeForwardSection")
    if(weight != 0.0)
    {
        uint8_t sideCounter;
        KyoChanForwardEdge* forwardEnd = currentSection->forwardEdges
                + currentSection->numberOfForwardEdges;
        for(KyoChanForwardEdge* forwardEdge = currentSection->forwardEdges;
            forwardEdge < forwardEnd;
            forwardEdge++)
        {
            if(forwardEdge->weight != 0.0)
            {
                KyoChanEdgeForwardContainer newEdge;
                newEdge.targetEdgeSectionId = forwardEdge->targetEdgeSectionId;
                newEdge.weight = forwardEdge->weight;

                outgoBuffer->addForwardEdge(sideCounter, &newEdge);
            }
            sideCounter++;
        }
    }
}

/**
 * @brief processEdgeSection
 * @param currentSection
 * @param weight
 * @param nodes
 * @param outgoBuffer
 */
void ClusterProcessing::processEdgeSection(KyoChanEdgeSection* currentSection,
                                               const float weight,
                                               KyoChanNode* nodes,
                                               OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processEdgeSection")
    if(weight != 0.0)
    {
        uint8_t sideCounter;
        KyoChanForwardEdge* forwardEnd = currentSection->forwardEdges
                + currentSection->numberOfForwardEdges;
        for(KyoChanForwardEdge* forwardEdge = currentSection->forwardEdges;
            forwardEdge < forwardEnd;
            forwardEdge++)
        {
            if(forwardEdge->weight != 0.0)
            {
                KyoChanEdgeForwardContainer newEdge;
                newEdge.targetEdgeSectionId = forwardEdge->targetEdgeSectionId;
                newEdge.weight = forwardEdge->weight * weight;

                outgoBuffer->addForwardEdge(sideCounter, &newEdge);
            }
            sideCounter++;
        }

        // process edge-section
        KyoChanEdge* end = currentSection->edges + currentSection->numberOfEdges;
        for(KyoChanEdge* edge = currentSection->edges;
            edge < end;
            edge++)
        {
            nodes[edge->targetNodeId].currentState += edge->weight * weight;
        }
    }
}

/**
 * @brief createNewEdgeForward
 * @param cluster
 * @param sourceEdgeClusterId
 * @param outgoBuffer
 * @param nextChooser
 */
inline void ClusterProcessing::createNewEdgeForward(EdgeCluster *cluster,
                                                        const uint32_t sourceEdgeClusterId,
                                                        OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("createNewEdgeForward")
    const uint8_t nextSide = m_nextChooser->getNextCluster(cluster->getNeighbors(), 14);

    // TODO: calculate a value
    const float weight = 100.0;

    KyoChanLearingEdgeContainer newEdge;
    newEdge.sourceEdgeSectionId = sourceEdgeClusterId;
    newEdge.weight = weight;

    outgoBuffer->addLearingEdge(nextSide, &newEdge);
}

/**
 * @brief CpuProcessingUnit::processIncomAxonEdge
 * @param data
 * @return
 */
inline void ClusterProcessing::processAxonEdge(uint8_t *data,
                                                   OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processIncomAxonEdge")
    KyoChanAxonEdgeContainer* edge = (KyoChanAxonEdgeContainer*)data;

    // check if target-cluster is reached
    if(edge->targetClusterPath != 0)
    {
        // if not reached update data
        uint8_t side = edge->targetClusterPath % 16;
        edge->targetClusterPath /= 16;

        // send edge to the next cluster
        outgoBuffer->addAxonEdge(side, edge);
    }
    else
    {
        // if target cluster reached, update the state of the target-axon with the edge
        //axon[edge->targetAxonId].currentState = edge->weight;
    }
}


/**
 * @brief EdgeProcessing::processIncomLerningEdge
 * @param data
 * @param initSide
 * @param outgoBuffer
 * @param edgeCluster
 */
inline void ClusterProcessing::processLerningEdge(uint8_t *data,
                                                      const uint8_t initSide,
                                                      EdgeCluster* cluster,
                                                      OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processIncomLerningEdge")
    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;

    const uint32_t targetEdgeSectionId = cluster->addEmptyForwardEdgeSection();

    if(targetEdgeSectionId != 0xFFFFFFFF)
    {
        // create reply-message
        KyoChanLearningEdgeReplyContainer reply;
        reply.sourceEdgeSectionId = edge->sourceEdgeSectionId;
        reply.targetEdgeSectionId = targetEdgeSectionId;

        // send reply-message
        outgoBuffer->addLearningReplyMessage(initSide, &reply);
    }
}

/**
 * @brief processPendingEdge
 * @param data
 * @param cluster
 * @param outgoBuffer
 */
inline void ClusterProcessing::processPendingEdge(uint8_t *data,
                                                      EdgeCluster* cluster,
                                                      OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processPendingEdge")
    KyoChanPendingEdgeContainer* edge = (KyoChanPendingEdgeContainer*)data;
}

/**
 * @brief processIncomForwardEdge
 * @param data
 * @param nodeCluster
 * @param outgoBuffer
 */
inline void ClusterProcessing::processForwardEdge(uint8_t *data,
                                                      EdgeCluster* cluster,
                                                      OutgoingMessageBuffer* outgoBuffer)
{
    KyoChanEdgeForwardContainer* edge = (KyoChanEdgeForwardContainer*)data;

    processEdgeForwardSection(&(((EdgeCluster*)cluster)->getForwardEdgeSectionBlock()[edge->targetEdgeSectionId]),
                              edge->weight,
                              outgoBuffer);
}

/**
 * @brief processIncomLearningReply
 * @param data
 * @param initSide
 * @param cluster
 */
inline void ClusterProcessing::processLearningReply(uint8_t *data,
                                                        uint8_t initSide,
                                                        EdgeCluster* cluster)
{
    KyoChanLearningEdgeReplyContainer* edge = (KyoChanLearningEdgeReplyContainer*)data;

    KyoChanForwardEdgeSection* edgeForwardSections = ((EdgeCluster*)cluster)->getForwardEdgeSectionBlock();
    edgeForwardSections[edge->sourceEdgeSectionId].forwardEdges[initSide].targetEdgeSectionId =
            edge->targetEdgeSectionId;
}

/**
 * @brief ClusterProcessing::processIncomDirectEdge
 * @param data
 * @param cluster
 */
inline void ClusterProcessing::processDirectEdge(uint8_t *data,
                                                     EdgeCluster *cluster)
{
    OUTPUT("---")
    OUTPUT("processDirectEdge")
    KyoChanDirectEdgeContainer* edge = (KyoChanDirectEdgeContainer*)data;
    if(cluster->getClusterType() == NODE_CLUSTER) {
        ((NodeCluster*)cluster)->getNodeBlock()[edge->targetNodeId].currentState += edge->weight;
    }
}

}
