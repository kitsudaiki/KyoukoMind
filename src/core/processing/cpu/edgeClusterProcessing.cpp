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

#include <messageQueues/incomingMessageBuffer.h>
#include <messageQueues/outgoingMessageBuffer.h>

#include <messages/message.h>
#include <messages/dataMessage.h>

#include "weightmap.h"

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
    // process normal communication
    std::vector<uint8_t> sideOrder = {0, 2, 3, 4, 8, 14, 13, 12};
    for(uint8_t sidePos = 0; sidePos < sideOrder.size(); sidePos++)
    {
        const uint8_t side = sideOrder[sidePos];

        // get buffer
        Kitsune::MindMessaging::IncomingMessageBuffer* incomBuffer = cluster->getIncomingMessageBuffer(side);

        if(incomBuffer == nullptr) {
            continue;
        }

        refillWeightMap(side, cluster->getNeighbors(), m_weightMap);

        Kitsune::MindMessaging::DataMessage* currentMessage = incomBuffer->getMessage();
        uint8_t* data = currentMessage->getPayload();
        uint8_t* end = data + currentMessage->getPayloadSize();

        while(data < end)
        {
            switch((int)(*data))
            {
                case STATUS_EDGE_CONTAINER:
                {
                    KyoChanStatusEdgeContainer* edge = (KyoChanStatusEdgeContainer*)data;
                    processStatusEdge(cluster, edge->targetId, edge->status, side);
                    data += sizeof(KyoChanStatusEdgeContainer);
                    break;
                }
                case PENDING_EDGE_CONTAINER:
                {
                    KyoChanPendingEdgeContainer* edge = (KyoChanPendingEdgeContainer*)data;
                    processPendingEdge(cluster, edge->sourceEdgeSectionId, edge->sourceSide, edge->weight);
                    data += sizeof(KyoChanPendingEdgeContainer);
                    break;
                }
                case FOREWARD_EDGE_CONTAINER:
                {
                    KyoChanForwardEdgeContainer* edge = (KyoChanForwardEdgeContainer*)data;
                    processEdgeForwardSection(cluster, edge->targetEdgeSectionId, edge->weight);
                    data += sizeof(KyoChanForwardEdgeContainer);
                    break;
                }
                case AXON_EDGE_CONTAINER:
                {
                    KyoChanAxonEdgeContainer* edge = (KyoChanAxonEdgeContainer*)data;
                    processAxon(cluster, edge->targetAxonId, edge->targetClusterPath, edge->weight);
                    data += sizeof(KyoChanAxonEdgeContainer);
                    break;
                }
                case LEARNING_EDGE_CONTAINER:
                {
                    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;
                    processLerningEdge(cluster, edge->sourceEdgeSectionId, edge->weight, side);
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
            }
        }
    }

    return true;
}

/**
 * @brief EdgeClusterProcessing::updateEdgeForwardSection
 * @param cluster
 * @param forwardEdgeSectionId
 * @param status
 * @param inititalSide
 */
inline void EdgeClusterProcessing::processStatusEdge(EdgeCluster *cluster,
                                                     const uint32_t forwardEdgeSectionId,
                                                     const float status,
                                                     const uint8_t inititalSide)
{
    KyoChanForwardEdgeSection* currentSection = &((cluster)->getForwardEdgeSectionBlock()[forwardEdgeSectionId]);
    currentSection->updateWeight(inititalSide, status);

    if(currentSection->numberOfActiveEdges == 0 || currentSection->sourceId == 0) {
        return;
    }

    KyoChanStatusEdgeContainer newEdge;
    newEdge.status = status;
    newEdge.targetId = currentSection->sourceId;
    cluster->getOutgoingMessageBuffer(currentSection->sourceSide)->addData(&newEdge);
}

/**
 * @brief EdgeClusterProcessing::processAxon
 * @param cluster
 * @param targetId
 * @param path
 * @param weight
 * @param initialSide
 */
inline void EdgeClusterProcessing::processAxon(EdgeCluster* cluster,
                                               const uint32_t targetId,
                                               const uint64_t path,
                                               const float weight)
{
    if(path != 0)
    {
        KyoChanAxonEdgeContainer newEdge;
        newEdge.targetClusterPath = path / 32;
        newEdge.weight = weight;
        newEdge.targetAxonId = targetId;
        cluster->getOutgoingMessageBuffer(path % 32)->addData(&newEdge);
    }
    else
    {
        // if target cluster reached, update the state of the target-axon with the edge
        processEdgeForwardSection(cluster, targetId, weight);
    }
}

/**
 * @brief EdgeClusterProcessing::processLerningEdge
 * @param cluster
 * @param sourceEdgeSectionId
 * @param weight
 * @param initSide
 */
inline void EdgeClusterProcessing::processLerningEdge(EdgeCluster* cluster,
                                                      const uint32_t sourceEdgeSectionId,
                                                      const float weight,
                                                      const uint8_t initSide)
{
    const uint32_t targetEdgeSectionId = cluster->addEmptyForwardEdgeSection(initSide,
                                                                             sourceEdgeSectionId);

    if(targetEdgeSectionId != UNINIT_STATE_32)
    {
        // create reply-message
        KyoChanLearningEdgeReplyContainer reply;
        reply.sourceEdgeSectionId = sourceEdgeSectionId;
        reply.targetEdgeSectionId = targetEdgeSectionId;
        cluster->getOutgoingMessageBuffer(initSide)->addData(&reply);

        processEdgeForwardSection(cluster, targetEdgeSectionId, weight);
    }
}

/**
 * @brief EdgeClusterProcessing::processPendingEdge
 * @param cluster
 * @param sourceId
 * @param sourceSide
 * @param weight
 */
inline void EdgeClusterProcessing::processPendingEdge(EdgeCluster *cluster,
                                                      const uint32_t sourceId,
                                                      const uint8_t sourceSide,
                                                      const float weight)
{
    KyoChanForwardEdgeSection* forwardEnd = &((cluster)->getForwardEdgeSectionBlock()[0]);
    const uint32_t numberOfForwardEdgeSections = cluster->getNumberOfForwardEdgeSections();
    KyoChanForwardEdgeSection* forwardStart = &((cluster)->getForwardEdgeSectionBlock()[numberOfForwardEdgeSections - 1]);

    uint32_t forwardEdgeSectionId = numberOfForwardEdgeSections - 1;
    for(KyoChanForwardEdgeSection* forwardEdgeSection = forwardStart;
        forwardEdgeSection >= forwardEnd;
        forwardEdgeSection--)
    {
        if(sourceId == forwardEdgeSection->sourceId
                && sourceSide == forwardEdgeSection->sourceSide)
        {
            processEdgeForwardSection(cluster, forwardEdgeSectionId, weight);
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
inline void EdgeClusterProcessing::learningForwardEdgeSection(EdgeCluster *cluster,
                                                              KyoChanForwardEdgeSection *currentSection,
                                                              const uint32_t forwardEdgeSectionId,
                                                              const float partitialWeight)
{
    if(partitialWeight < 1.0f) {
        return;
    }

    for(uint8_t side = 0; side < 17; side++)
    {
        const float currentSideWeight = partitialWeight * m_weightMap[side];

        // set a border to avoid too many new edges
        if(m_weightMap[side] <= NEW_FORWARD_EDGE_BORDER
                || currentSideWeight <= 1.0f) {
            continue;
        }

        // cluster-external lerning
        if(currentSection->forwardEdges[side].targetId == UNINIT_STATE_32
                && currentSection->forwardEdges[side].weight == 0.0f
                && side != 16)
        {
            // send new learning-edge
            KyoChanLearingEdgeContainer newEdge;
            newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
            newEdge.weight = currentSideWeight;
            cluster->getOutgoingMessageBuffer(side)->addData(&newEdge);
        }

        currentSection->forwardEdges[side].weight += currentSideWeight;
        currentSection->totalWeight += currentSideWeight;
    }
}

/**
  * @brief EdgeClusterProcessing::processEdgeForwardSection
  * @param currentSection
  * @param weight
  */
inline void EdgeClusterProcessing::processEdgeForwardSection(EdgeCluster *cluster,
                                                             uint32_t forwardEdgeSectionId,
                                                             const float weight)
{
    if(weight != 0.0f)
    {
        KyoChanForwardEdgeSection* currentSection = &((cluster)->getForwardEdgeSectionBlock()[forwardEdgeSectionId]);

        // learning
        if(weight - currentSection->totalWeight > 1.0f)
        {
            learningForwardEdgeSection(cluster,
                                       currentSection,
                                       forwardEdgeSectionId,
                                       weight - currentSection->totalWeight);
        }

        float ratio = currentSection->totalWeight / weight;
        if(ratio > 1.0f) {
            ratio = 1.0f;
        }

        // normal processing
        uint8_t sideCounter = 2; // to skip side number 0 and 1
        KyoChanForwardEdge* forwardStart = currentSection->forwardEdges + sideCounter;
        KyoChanForwardEdge* forwardEnd = currentSection->forwardEdges + 17;

        for(KyoChanForwardEdge* forwardEdge = forwardStart;
            forwardEdge < forwardEnd;
            forwardEdge++)
        {
            const KyoChanForwardEdge tempForwardEdge = *forwardEdge;

            if(forwardEdge->weight != 0.0f)
            {
                if(sideCounter != 16)
                {
                    if(forwardEdge->targetId != UNINIT_STATE_32)
                    {
                        // normal external edge
                        KyoChanForwardEdgeContainer newEdge;
                        newEdge.targetEdgeSectionId = tempForwardEdge.targetId;
                        newEdge.weight = tempForwardEdge.weight * ratio;
                        cluster->getOutgoingMessageBuffer(sideCounter)->addData(&newEdge);
                    }
                    else
                    {
                        KyoChanPendingEdgeContainer newEdge;
                        newEdge.weight = tempForwardEdge.weight * ratio;
                        newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
                        newEdge.sourceSide = 16 - sideCounter;
                        cluster->getOutgoingMessageBuffer(sideCounter)->addData(&newEdge);
                    }
                }
                else
                {
                    KyoChanDirectEdgeContainer newEdge;
                    newEdge.targetNodeId = 0;
                    newEdge.weight = tempForwardEdge.weight * ratio;
                    cluster->getOutgoingMessageBuffer(sideCounter)->addData(&newEdge);
                }
            }

            sideCounter++;
        }
    }
}

}
