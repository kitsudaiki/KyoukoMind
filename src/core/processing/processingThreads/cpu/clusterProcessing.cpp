#include "clusterProcessing.h"

namespace KyoukoMind
{

/**
 * @brief ClusterProcessing::ClusterProcessing
 * @param nextChooser
 */
ClusterProcessing::ClusterProcessing(NextChooser* nextChooser)
{
    m_sideOrder = {2,3,4,13,12,11};
    m_nextChooser = nextChooser;
}


/**
 * @brief AxonProcessing::processAxons
 * @param cluster
 * @return
 */
bool ClusterProcessing::processAxons(Cluster* cluster)
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
        if(cluster->getClusterId() == EDGE_CLUSTER)
        {
            KyoChanForwardEdgeSection* edgeForwardSections = ((EdgeCluster*)cluster)->getEdgeBlock();
            processEdgeForwardSection(&edgeForwardSections[axon->edgeSectionId],
                                      axon->currentState,
                                      outgoBuffer);
        }
        else
        {
            KyoChanEdgeSection* edgeSections = ((NodeCluster*)cluster)->getEdgeBlock();
            processEdgeSection(&edgeSections[axon->edgeSectionId],
                               axon->currentState,
                               ((NodeCluster*)cluster)->getNodeBlock(),
                               outgoBuffer);
        }

    }
    return true;
}

/**
 * @brief EdgeProcessing::processIncomingMessages
 * @param edgeCluster
 * @return
 */
bool ClusterProcessing::processMessagesEdges(Cluster* cluster)
{
    IncomingMessageBuffer* incomBuffer = cluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = cluster->getOutgoingMessageBuffer();

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
                    processIncomDirectEdge(data, cluster);
                    data += sizeof(KyoChanDirectEdgeContainer);
                    break;
                case FOREWARD_EDGE_CONTAINER:
                    processForwardEdge(data, cluster, outgoBuffer);
                    data += sizeof(KyoChanEdgeForwardContainer);
                    break;
                case AXON_EDGE_CONTAINER:
                    processAxonEdge(data, cluster->getAxonBlock(), outgoBuffer);
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
            std::cout<<"        side: "<<(int)sideCounter<<std::endl;
            std::cout<<"        forwardEdge->weight: "<<forwardEdge->weight<<std::endl;
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
    std::cout<<"    weight: "<<weight<<std::endl;
    if(weight != 0.0)
    {
        uint8_t sideCounter;
        KyoChanForwardEdge* forwardEnd = currentSection->forwardEdges
                + currentSection->numberOfForwardEdges;
        for(KyoChanForwardEdge* forwardEdge = currentSection->forwardEdges;
            forwardEdge < forwardEnd;
            forwardEdge++)
        {
            std::cout<<"        side: "<<(int)sideCounter<<std::endl;
            std::cout<<"        forwardEdge->weight: "<<forwardEdge->weight<<std::endl;
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
void ClusterProcessing::createNewEdgeForward(Cluster *cluster,
                                                    const uint32_t sourceEdgeClusterId,
                                                    OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("createNewEdgeForward")
    const uint8_t nextSide = m_nextChooser->getNextCluster(cluster->getNeighbors(), 14);
    const uint32_t newEdgeId = cluster->getNextNewEdgeId();

    // TODO: calculate a value
    const float weight = 100.0;

    KyoChanLearingEdgeContainer newEdge;
    newEdge.marker = newEdgeId;
    newEdge.sourceEdgeSectionId = sourceEdgeClusterId;
    newEdge.weight = weight;

    outgoBuffer->addLearingEdge(nextSide, &newEdge);
}

/**
 * @brief CpuProcessingUnit::processIncomAxonEdge
 * @param data
 * @return
 */
void ClusterProcessing::processAxonEdge(uint8_t *data,
                                               KyoChanAxon* axon,
                                               OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processIncomAxonEdge")
    KyoChanAxonEdgeContainer* edge = (KyoChanAxonEdgeContainer*)data;

    // check if target-cluster is reached
    std::cout<<"    edge->targetClusterPath: "<<edge->targetClusterPath<<std::endl;
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
        axon[edge->targetAxonId].currentState = edge->weight;
    }
}


/**
 * @brief EdgeProcessing::processIncomLerningEdge
 * @param data
 * @param initSide
 * @param outgoBuffer
 * @param edgeCluster
 */
void ClusterProcessing::processLerningEdge(uint8_t *data,
                                                  const uint8_t initSide,
                                                  Cluster* cluster,
                                                  OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processIncomLerningEdge")
    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;

    const uint32_t targetEdgeSectionId = cluster->addEmptyEdgeSection();

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
void ClusterProcessing::processPendingEdge(uint8_t *data,
                                                  Cluster* cluster,
                                                  OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processPendingEdge")
    KyoChanPendingEdgeContainer* edge = (KyoChanPendingEdgeContainer*)data;
}

}
