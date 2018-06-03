/**
 *  @file    clusterProcessing.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "edgeClusterProcessing.h"

#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/messaging/messageController.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messages/replyMessage.h>

#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

namespace KyoukoMind
{

/**
 * @brief EdgeClusterProcessing::ClusterProcessing
 * @param nextChooser
 */
EdgeClusterProcessing::EdgeClusterProcessing(PossibleKyoChanNodes* activeNodes)
{
    m_activeNodes = activeNodes;
}


/**
 * @brief EdgeClusterProcessing::processIncomingMessages processing of all incoming messages in a cluster
 * @param edgeCluster custer which should be processed
 * @return false if a message-type does not exist, else true
 */
bool EdgeClusterProcessing::processMessagesEdgesCluster(EdgeCluster* cluster)
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
                    updateEdgeForwardSection(cluster,
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
                    processEdgeForwardSection(cluster,
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
                    processAxon(cluster,
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
                    processLerningEdge(cluster,
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
 * @brief EdgeClusterProcessing::randFloat
 * @param b
 * @return
 */
inline float EdgeClusterProcessing::randFloat(const float b)
{
    const float random = ((float) rand()) / (float) UNINIT_STATE;
    return random * b;
}

/**
 * @brief EdgeClusterProcessing::updateEdgeForwardSection
 * @param cluster
 * @param forwardEdgeSectionId
 * @param status
 * @param inititalSide
 * @param outgoBuffer
 */
inline void EdgeClusterProcessing::updateEdgeForwardSection(EdgeCluster *cluster,
                                                            const uint32_t forwardEdgeSectionId,
                                                            const float status,
                                                            const uint8_t inititalSide,
                                                            OutgoingMessageBuffer *outgoBuffer)
{
    std::cout<<"---"<<std::endl;
    std::cout<<"updateEdgeForwardSection"<<std::endl;
    KyoChanForwardEdgeSection* currentSection = &((cluster)->getForwardEdgeSectionBlock()[forwardEdgeSectionId]);
    //currentSection->forwardEdges[inititalSide].updateMemorize(status);

    if(currentSection->numberOfActiveEdges == 0 || currentSection->sourceId == 0) {
        return;
    }

    KyoChanStatusEdgeContainer newEdge;
    newEdge.status = status / currentSection->numberOfActiveEdges;
    newEdge.targetId = currentSection->sourceId;
    outgoBuffer->addStatusEdge(currentSection->sourceSide, &newEdge);
}

/**
 * @brief EdgeClusterProcessing::processAxon
 * @param cluster
 * @param targetId
 * @param path
 * @param weight
 * @param initialSide
 * @param outgoBuffer
 */
inline void EdgeClusterProcessing::processAxon(EdgeCluster* cluster,
                                               const uint32_t targetId,
                                               const uint64_t path,
                                               const float weight,
                                               const uint8_t initialSide,
                                               OutgoingMessageBuffer* outgoBuffer)
{
    if(path != 0)
    {
        KyoChanAxonEdgeContainer newEdge;
        newEdge.targetClusterPath = path / 17;
        newEdge.weight = weight;
        newEdge.targetAxonId = targetId;
        outgoBuffer->addAxonEdge(path % 17, &newEdge);
    }
    else
    {
        // if target cluster reached, update the state of the target-axon with the edge
        processEdgeForwardSection(cluster,
                                  targetId,
                                  weight,
                                  initialSide,
                                  outgoBuffer);
    }
}

/**
 * @brief EdgeClusterProcessing::processLerningEdge
 * @param cluster
 * @param sourceEdgeSectionId
 * @param weight
 * @param initSide
 * @param outgoBuffer
 */
inline void EdgeClusterProcessing::processLerningEdge(EdgeCluster* cluster,
                                                      const uint32_t sourceEdgeSectionId,
                                                      const float weight,
                                                      const uint8_t initSide,
                                                      OutgoingMessageBuffer* outgoBuffer)
{
    const uint32_t targetEdgeSectionId = cluster->addEmptyForwardEdgeSection(initSide,
                                                                             sourceEdgeSectionId);
    processEdgeForwardSection(cluster,
                              targetEdgeSectionId,
                              weight,
                              initSide,
                              outgoBuffer);

    if(targetEdgeSectionId != UNINIT_STATE)
    {
        // create reply-message
        KyoChanLearningEdgeReplyContainer reply;
        reply.sourceEdgeSectionId = sourceEdgeSectionId;
        reply.targetEdgeSectionId = targetEdgeSectionId;

        // send reply-message
        outgoBuffer->addLearningReplyMessage(initSide, &reply);
    }
}

/**
 * @brief EdgeClusterProcessing::learningForwardEdgeSection
 * @param currentSection
 * @param side
 * @param partitialWeight
 * @param outgoBuffer
 */
inline void EdgeClusterProcessing::learningForwardEdgeSection(EdgeCluster* cluster,
                                                              KyoChanForwardEdgeSection* currentSection,
                                                              const uint32_t forwardEdgeSectionId,
                                                              const uint8_t inititalSide,
                                                              const float partitialWeight,
                                                              OutgoingMessageBuffer* outgoBuffer)
{
    std::cout<<"---"<<std::endl;
    std::cout<<"learningForwardEdgeSection"<<std::endl;
    // get side of learning-process

    // TODO
    const uint8_t nextSide = 0;

    // cluster-external lerning
    if(currentSection->forwardEdges[nextSide].targetId == UNINIT_STATE
            && nextSide != 8
            && nextSide != 16)
    {
        // send new learning-edge
        KyoChanLearingEdgeContainer newEdge;
        newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
        newEdge.weight = partitialWeight;
        outgoBuffer->addLearingEdge(nextSide, &newEdge);

        currentSection->setPedingBit(nextSide);
    }

    if(currentSection->forwardEdges[nextSide].targetId == UNINIT_STATE
            && nextSide == 16)
    {
        // send new learning-edge
        KyoChanDirectEdgeContainer newEdge;
        newEdge.targetNodeId = 0;
        newEdge.weight = partitialWeight;
        outgoBuffer->addDirectEdge(nextSide, &newEdge);

        currentSection->setPedingBit(nextSide);
    }

    currentSection->updateWeight(nextSide, partitialWeight);
}

/**
  * @brief EdgeClusterProcessing::processEdgeForwardSection
  * @param currentSection
  * @param weight
  * @param inititalSide
  * @param outgoBuffer
  */
inline  void EdgeClusterProcessing::processEdgeForwardSection(EdgeCluster *cluster,
                                                              uint32_t forwardEdgeSectionId,
                                                              const float weight,
                                                              const uint8_t inititalSide,
                                                              OutgoingMessageBuffer* outgoBuffer)
{
    std::cout<<"---"<<std::endl;
    std::cout<<"processEdgeForwardSection"<<std::endl;
    KyoChanForwardEdgeSection* currentSection = &((cluster)->getForwardEdgeSectionBlock()[forwardEdgeSectionId]);
    bool processPendingEdge = false;

    if(forwardEdgeSectionId == UNINIT_STATE) {
        forwardEdgeSectionId = cluster->getPendingForwardEdgeSectionId();
        processPendingEdge = true;
    }
    if(weight != 0.0 || currentSection->pendingEdges != 0)
    {
        // learning
        if(weight > currentSection->totalWeight) {
            learningForwardEdgeSection(cluster,
                                       currentSection,
                                       forwardEdgeSectionId,
                                       inititalSide,
                                       weight - currentSection->totalWeight,
                                       outgoBuffer);
        }

        const float ratio = currentSection->totalWeight / weight;
        // normal processing
        uint8_t sideCounter = 2; // to skip side number 0 and 1
        KyoChanForwardEdge* forwardStart = currentSection->forwardEdges + sideCounter;
        KyoChanForwardEdge* forwardEnd = currentSection->forwardEdges + 17;

        for(KyoChanForwardEdge* forwardEdge = forwardStart;
            forwardEdge < forwardEnd;
            forwardEdge++)
        {
            const KyoChanForwardEdge tempForwardEdge = *forwardEdge;

            if(forwardEdge->weight != 0.0 && forwardEdge->targetId != UNINIT_STATE)
            {
                // normal external edge
                KyoChanForwardEdgeContainer newEdge;
                newEdge.targetEdgeSectionId = tempForwardEdge.targetId;
                newEdge.weight = tempForwardEdge.weight * ratio;
                outgoBuffer->addForwardEdge(sideCounter, &newEdge);
            }

            const float diff = tempForwardEdge.weight * (1.0f - tempForwardEdge.memorize);
            forwardEdge->weight *= tempForwardEdge.memorize;
            currentSection->totalWeight -= diff;

            currentSection->zeroPendingBit(sideCounter);
            sideCounter++;
        }
    }

    if(processPendingEdge) {
        cluster->decreaseNumberOfPendingForwardEdges();
    }
}

}
