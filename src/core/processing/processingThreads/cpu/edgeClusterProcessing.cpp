/**
 *  @file    edgeClusterProcessing.cpp
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
 */
EdgeClusterProcessing::EdgeClusterProcessing()
{
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

    // process normal communication
    std::vector<uint8_t> sideOrder = {0, 2, 3, 4, 8, 14, 13, 12};
    for(uint8_t sidePos = 0; sidePos < sideOrder.size(); sidePos++)
    {
        const uint8_t side = sideOrder[sidePos];

        refillWeightMap(side, cluster->getNeighbors());

        uint8_t* data = (uint8_t*)incomBuffer->getMessage(side)->getPayload();
        uint8_t* end = data + incomBuffer->getMessage(side)->getPayloadSize();

        while(data < end)
        {
            switch((int)(*data))
            {
                case STATUS_EDGE_CONTAINER:
                {
                    KyoChanStatusEdgeContainer* edge = (KyoChanStatusEdgeContainer*)data;
                    updateEdgeForwardSection(cluster, edge->targetId, edge->status, side, outgoBuffer);
                    data += sizeof(KyoChanStatusEdgeContainer);
                    break;
                }
                case PENDING_EDGE_CONTAINER:
                {
                    KyoChanPendingEdgeContainer* edge = (KyoChanPendingEdgeContainer*)data;
                    processPendingEdge(cluster, edge->sourceEdgeSectionId, edge->sourceSide, edge->weight, outgoBuffer);
                    data += sizeof(KyoChanPendingEdgeContainer);
                    break;
                }
                case FOREWARD_EDGE_CONTAINER:
                {
                    KyoChanForwardEdgeContainer* edge = (KyoChanForwardEdgeContainer*)data;
                    processEdgeForwardSection(cluster, edge->targetEdgeSectionId, edge->weight, outgoBuffer);
                    data += sizeof(KyoChanForwardEdgeContainer);
                    break;
                }
                case AXON_EDGE_CONTAINER:
                {
                    KyoChanAxonEdgeContainer* edge = (KyoChanAxonEdgeContainer*)data;
                    processAxon(cluster, edge->targetAxonId, edge->targetClusterPath, edge->weight, outgoBuffer);
                    data += sizeof(KyoChanAxonEdgeContainer);
                    break;
                }
                case LEARNING_EDGE_CONTAINER:
                {
                    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;
                    processLerningEdge(cluster, edge->sourceEdgeSectionId, edge->weight, side, outgoBuffer);
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
        processEdgeForwardSection(cluster, targetId, weight, outgoBuffer);
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
    processEdgeForwardSection(cluster, targetEdgeSectionId, weight, outgoBuffer);

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
 * @brief EdgeClusterProcessing::processPendingEdge
 * @param cluster
 * @param sourceId
 * @param sourceSide
 * @param weight
 * @param outgoBuffer
 */
inline void EdgeClusterProcessing::processPendingEdge(EdgeCluster *cluster,
                                                      const uint32_t sourceId,
                                                      const uint8_t sourceSide,
                                                      const float weight,
                                                      OutgoingMessageBuffer *outgoBuffer)
{
    KyoChanForwardEdgeSection* forwardEnd = &((cluster)->getForwardEdgeSectionBlock()[0]);
    const uint32_t numberOfForwardEdgeBlocks = cluster->getNumberOfForwardEdgeSections();
    KyoChanForwardEdgeSection* forwardStart = &((cluster)->getForwardEdgeSectionBlock()[numberOfForwardEdgeBlocks - 1]);

    uint32_t forwardEdgeSectionId = numberOfForwardEdgeBlocks - 1;
    for(KyoChanForwardEdgeSection* forwardEdgeSection = forwardStart;
        forwardEdgeSection >= forwardEnd;
        forwardEdgeSection--)
    {
        if(sourceId == forwardEdgeSection->sourceId
                && sourceSide == forwardEdgeSection->sourceSide)
        {
            processEdgeForwardSection(cluster, forwardEdgeSectionId, weight, outgoBuffer);
        }
        forwardEdgeSectionId--;
    }
}

/**
 * @brief EdgeClusterProcessing::learningForwardEdgeSection
 * @param currentSection
 * @param forwardEdgeSectionId
 * @param partitialWeight
 * @param outgoBuffer
 */
inline void EdgeClusterProcessing::learningForwardEdgeSection(KyoChanForwardEdgeSection *currentSection,
                                                              const uint32_t forwardEdgeSectionId,
                                                              const float partitialWeight,
                                                              OutgoingMessageBuffer* outgoBuffer)
{
    std::cout<<"---"<<std::endl;
    std::cout<<"learningForwardEdgeSection"<<std::endl;

    for(uint8_t side = 0; side < 17; side++)
    {
        const float currentSideWeight = partitialWeight * m_weightMap[side];

        if(currentSideWeight <= NEW_FORWARD_EDGE_BORDER * partitialWeight
                && side != 16) {
            continue;
        }

        currentSection->updateWeight(side, currentSideWeight);

        // cluster-external lerning
        if(currentSection->forwardEdges[side].targetId == UNINIT_STATE
                && side != 16)
        {
            // send new learning-edge
            KyoChanLearingEdgeContainer newEdge;
            newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
            newEdge.weight = currentSideWeight;
            outgoBuffer->addLearingEdge(side, &newEdge);
        }
    }
}

/**
  * @brief EdgeClusterProcessing::processEdgeForwardSection
  * @param currentSection
  * @param weight
  * @param outgoBuffer
  */
inline void EdgeClusterProcessing::processEdgeForwardSection(EdgeCluster *cluster,
                                                             uint32_t forwardEdgeSectionId,
                                                             const float weight,
                                                             OutgoingMessageBuffer* outgoBuffer)
{
    std::cout<<"---"<<std::endl;
    std::cout<<"processEdgeForwardSection"<<std::endl;

    if(weight != 0.0)
    {
        KyoChanForwardEdgeSection* currentSection = &((cluster)->getForwardEdgeSectionBlock()[forwardEdgeSectionId]);

        // learning
        if(weight > currentSection->totalWeight) {
            learningForwardEdgeSection(currentSection,
                                       forwardEdgeSectionId,
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

            if(forwardEdge->weight != 0.0)
            {
                if(sideCounter != 16)
                {
                    if(forwardEdge->targetId != UNINIT_STATE)
                    {
                        // normal external edge
                        KyoChanForwardEdgeContainer newEdge;
                        newEdge.targetEdgeSectionId = tempForwardEdge.targetId;
                        newEdge.weight = tempForwardEdge.weight * ratio;
                        outgoBuffer->addForwardEdge(sideCounter, &newEdge);
                    }
                    else
                    {
                        KyoChanPendingEdgeContainer newEdge;
                        newEdge.weight = tempForwardEdge.weight * ratio;
                        newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
                        newEdge.sourceSide = 16 - sideCounter;
                        outgoBuffer->addPendingEdge(sideCounter, &newEdge);
                    }
                }
                else
                {
                    if(forwardEdge->targetId != UNINIT_STATE)
                    {
                        KyoChanDirectEdgeContainer newEdge;
                        newEdge.targetNodeId = 0;
                        newEdge.weight = tempForwardEdge.weight * ratio;
                        outgoBuffer->addDirectEdge(sideCounter, &newEdge);
                    }
                }
            }

            // update memorize
            const float diff = tempForwardEdge.weight * (1.0f - tempForwardEdge.memorize);
            forwardEdge->weight *= tempForwardEdge.memorize;
            currentSection->totalWeight -= diff;

            sideCounter++;
        }
    }
}

/**
 * @brief EdgeClusterProcessing::refillWeightMap
 * @param initialSide
 * @param neighbors
 */
inline void EdgeClusterProcessing::refillWeightMap(const uint8_t initialSide, Neighbor *neighbors)
{
    std::vector<uint8_t> possibleSides;

    // cleara existing map
    for(uint8_t side = 0; side < 17; side++)
    {
        m_weightMap[side] = 0.0;
    }

    // get possible next
    for(uint8_t side = 0; side < 17; side++)
    {
        if(neighbors[side].targetClusterId != UNINIT_STATE
                && side != initialSide) {
            possibleSides.push_back(side);
        }
    }

    // share weights
    for(uint8_t i = 0; i < 10; i++)
    {
        const uint8_t side = possibleSides[rand() % possibleSides.size()];
        m_weightMap[side] += 0.1;
    }
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

}
