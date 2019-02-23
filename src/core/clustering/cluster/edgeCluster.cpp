/**
 *  @file    edgeCluster.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/clustering/cluster/edgeCluster.h>
#include <core/common/weightmap.h>

#include <core/messaging/incomingMessageBuffer.h>
#include <core/messaging/outgoingMessageBuffer.h>

#include <kyoChanNetwork.h>
#include <commonDataBuffer.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>

namespace KyoukoMind
{

/**
 * @brief EdgeCluster::EdgeCluster constructor for edge-cluster-creation
 * @param clusterId id for the new edge-cluster
 * @param directoryPath path to the directory, where the file of the cluster should be saved
 */
EdgeCluster::EdgeCluster(const ClusterID &clusterId,
                         const uint32_t x,
                         const uint32_t y,
                         const std::string directoryPath) :
    Cluster(clusterId,
            x,
            y,
            directoryPath,
            0,
            sizeof(KyoChanForwardEdgeSection))
{
    // set meta-data
    m_metaData.clusterType = EDGE_CLUSTER;
    m_metaData.positionOfDynamicBlocks = 1;
}

#ifdef RUN_UNIT_TEST
/**
 * @brief EdgeCluster::getForwardEdgeSectionBlock_public
 * @return
 */
KyoChanForwardEdgeSection *EdgeCluster::getForwardEdgeSectionBlock_public()
{
    return getForwardEdgeSectionBlock();
}
#endif

/**
 * @brief EdgeCluster::getNumberOfForwardEdgeSectionBlocks get the number of forward-edge-section-blocks from meta-data
 * @return number of forward-edge-section-blocks
 */
uint32_t
EdgeCluster::getNumberOfForwardEdgeSectionBlocks() const
{
    return m_metaData.numberOfDynamicBlocks;
}

/**
 * @brief EdgeCluster::getNumberOfForwardEdgeSections get the number of forward-edge-sections from meta-data
 * @return number of forward-edge-sections
 */
uint32_t
EdgeCluster::getNumberOfForwardEdgeSections() const
{
    return m_metaData.numberOfDynamicItems;
}

/**
 * @brief EdgeCluster::getForwardEdgeSectionBlock get the forward-edge-section-block
 * @return pointer to the beginning of the forward-edge-section-block
 */
inline KyoChanForwardEdgeSection*
EdgeCluster::getForwardEdgeSectionBlock()
{
    const uint32_t position = m_metaData.positionOfDynamicBlocks;
    return (KyoChanForwardEdgeSection*)m_clusterDataBuffer->getBlock(position);
}

/**
 * @brief EdgeCluster::initForwardEdgeSectionBlocks initialize forward-edge-block
 * @param numberOfForwardEdgeSections number of forward-edge-sections
 * @return true if success, else false
 */
bool
EdgeCluster::initForwardEdgeSectionBlocks(const uint32_t numberOfForwardEdgeSections)
{
    if(initDynamicBlocks(numberOfForwardEdgeSections) == false) {
        return false;
    }

    // fill array with empty forward-edge-sections
    KyoChanForwardEdgeSection* array = getForwardEdgeSectionBlock();
    for(uint32_t i = 0; i < numberOfForwardEdgeSections; i++)
    {
        KyoChanForwardEdgeSection newSection;
        array[i] = newSection;
    }

    return true;
}

/**
 * @brief EdgeCluster::addEmptyForwardEdgeSection add a new forward-edge-section
 * @param sourceSide id of the incoming side
 * @param sourceId id of the source forward-edge-section
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
uint32_t
EdgeCluster::addEmptyForwardEdgeSection(const uint8_t sourceSide,
                                        const uint32_t sourceId)
{
    const uint32_t position = reserveDynamicItem();

    // add new edge-forward-section
    KyoChanForwardEdgeSection newSection;
    newSection.sourceId = sourceId;
    newSection.sourceSide = sourceSide;
    getForwardEdgeSectionBlock()[position] = newSection;

    return position;
}

/**
 * @brief EdgeCluster::processIncomingMessages processing of all incoming messages in a cluster
 * @return false if a message-type does not exist, else true
 */
bool
EdgeCluster::processEdgesCluster()
{
    bool result = true;
    initCycle();

    // process normal communication
    for(uint8_t sidePos = 0; sidePos < m_numberOfIncomingMessages; sidePos++)
    {
        const uint8_t side = m_incomingMessages[sidePos].second;

        #ifdef RUN_UNIT_TEST
        refillWeightMap(side, &m_metaData.neighors[0], m_weightMap, m_fakeRandVal);
        #else
        refillWeightMap(side, &m_metaData.neighors[0], m_weightMap);
        #endif

        // get and check message
        DataMessage* currentMessage = m_incomingMessages[sidePos].first;

        // get start and end of the message-payload
        uint8_t* data = currentMessage->getPayload();
        uint8_t* end = data + currentMessage->getPayloadSize();

        while(data < end)
        {
            switch((int)(*data))
            {
                // ------------------------------------------------------------------------------------------
                case STATUS_EDGE_CONTAINER:
                {
                    KyoChanUpdateEdgeContainer* edge = (KyoChanUpdateEdgeContainer*)data;
                    data += sizeof(KyoChanUpdateEdgeContainer);
                    if(edge->targetId == UNINIT_STATE_32 && edge->updateValue >= 0.0f) {
                        continue;
                    }
                    processUpdateEdge(edge->targetId, edge->updateValue, edge->updateType, side);
                    break;
                }
                // ------------------------------------------------------------------------------------------
                case PENDING_EDGE_CONTAINER:
                {
                    KyoChanPendingEdgeContainer* edge = (KyoChanPendingEdgeContainer*)data;
                    data += sizeof(KyoChanPendingEdgeContainer);
                    if(edge->sourceEdgeSectionId == UNINIT_STATE_32 && edge->weight >= 0.0f) {
                        continue;
                    }
                    processPendingEdge(edge->sourceEdgeSectionId, edge->sourceSide, edge->weight);
                    break;
                }
                // ------------------------------------------------------------------------------------------
                case FOREWARD_EDGE_CONTAINER:
                {
                    KyoChanForwardEdgeContainer* edge = (KyoChanForwardEdgeContainer*)data;
                    data += sizeof(KyoChanForwardEdgeContainer);
                    if(edge->targetEdgeSectionId == UNINIT_STATE_32 && edge->weight > 0.0f) {
                        continue;
                    }
                    processEdgeForwardSection(edge->targetEdgeSectionId, edge->weight);
                    break;
                }
                // ------------------------------------------------------------------------------------------
                case AXON_EDGE_CONTAINER:
                {
                    KyoChanAxonEdgeContainer* edge = (KyoChanAxonEdgeContainer*)data;
                    data += sizeof(KyoChanAxonEdgeContainer);
                    if(edge->targetAxonId == UNINIT_STATE_32 && edge->weight >= 0.0f) {
                        continue;
                    }
                    processAxon(edge->targetAxonId, edge->targetClusterPath, edge->weight);
                    break;
                }
                // ------------------------------------------------------------------------------------------
                case LEARNING_EDGE_CONTAINER:
                {
                    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;
                    data += sizeof(KyoChanLearingEdgeContainer);
                    if(edge->sourceEdgeSectionId == UNINIT_STATE_32 && edge->weight >= 0.0f) {
                        continue;
                    }
                    processLerningEdge(edge->sourceEdgeSectionId, edge->weight, side);
                    break;
                }
                // ------------------------------------------------------------------------------------------
                case LEARNING_REPLY_EDGE_CONTAINER:
                {
                    KyoChanLearningEdgeReplyContainer* edge = (KyoChanLearningEdgeReplyContainer*)data;
                    data += sizeof(KyoChanLearningEdgeReplyContainer);

                    KyoChanForwardEdgeSection* edgeForwardSections = getForwardEdgeSectionBlock();
                    if(edge->sourceEdgeSectionId == UNINIT_STATE_32) {
                        continue;
                    }
                    if(edgeForwardSections[edge->sourceEdgeSectionId].forwardEdges[side].weight == 0.0f) {
                        continue;
                    }
                    edgeForwardSections[edge->sourceEdgeSectionId].forwardEdges[side].targetId =
                            edge->targetEdgeSectionId;
                    break;
                }
                // ------------------------------------------------------------------------------------------
                default:
                    result = false;
            }
        }
    }

    finishCycle();
    return result;
}

/**
 * @brief EdgeCluster::checkAndDelete
 * @param currentSection
 * @param forwardEdgeSectionId
 */
bool
EdgeCluster::checkAndDelete(KyoChanForwardEdgeSection *currentSection,
                            const uint32_t forwardEdgeSectionId)
{
    if(currentSection->sourceId != UNINIT_STATE_32
            && currentSection->getActiveEdges() == 0)
    {
        KyoChanUpdateEdgeContainer newEdge;
        newEdge.updateType = KyoChanUpdateEdgeContainer::DELETE_TYPE;

        if(currentSection->sourceId != UNINIT_STATE_32)
        {
            newEdge.targetId = currentSection->sourceId;
            m_metaData.neighors[currentSection->sourceSide].outgoBuffer->addData(&newEdge);
        }
        deleteDynamicItem(forwardEdgeSectionId);

        return true;
    }
    return false;
}

/**
 * @brief EdgeCluster::processUpdateSetEdge
 * @param currentSection
 * @param updateValue
 * @param inititalSide
 */
inline void
EdgeCluster::processUpdateSetEdge(KyoChanForwardEdgeSection *currentSection,
                                  const float updateValue,
                                  const uint8_t inititalSide)
{
    KyoChanUpdateEdgeContainer newEdge;
    newEdge.updateValue = updateValue;
    currentSection->forwardEdges[inititalSide].weight = updateValue;

    if(currentSection->sourceId != UNINIT_STATE_32)
    {
        newEdge.targetId = currentSection->sourceId;
        m_metaData.neighors[currentSection->sourceSide].outgoBuffer->addData(&newEdge);
    }
}

/**
 * @brief EdgeCluster::processUpdateSubEdge
 * @param currentSection
 * @param updateValue
 * @param inititalSide
 */
inline void
EdgeCluster::processUpdateSubEdge(KyoChanForwardEdgeSection* currentSection,
                                  float updateValue,
                                  const uint8_t inititalSide)
{
    KyoChanUpdateEdgeContainer newEdge;
    newEdge.updateValue = updateValue;

    currentSection->forwardEdges[inititalSide].weight -= updateValue;

    KyoChanUpdateEdgeContainer replyEdge;
    replyEdge.updateType = KyoChanUpdateEdgeContainer::SET_TYPE;
    replyEdge.targetId = currentSection->forwardEdges[inititalSide].targetId;
    replyEdge.updateValue = currentSection->forwardEdges[inititalSide].weight;

    m_metaData.neighors[inititalSide].outgoBuffer->addData(&replyEdge);

    if(currentSection->sourceId != UNINIT_STATE_32)
    {
        newEdge.targetId = currentSection->sourceId;
        m_metaData.neighors[currentSection->sourceSide].outgoBuffer->addData(&newEdge);
    }
}

/**
 * @brief EdgeCluster::processUpdateDeleteEdge
 * @param currentSection
 * @param inititalSide
 */
inline void
EdgeCluster::processUpdateDeleteEdge(KyoChanForwardEdgeSection *currentSection,
                                     const uint32_t forwardEdgeSectionId,
                                     const uint8_t inititalSide)
{
    KyoChanUpdateEdgeContainer newEdge;
    newEdge.updateValue = currentSection->forwardEdges[inititalSide].weight;

    currentSection->forwardEdges[inititalSide].weight = 0.0f;
    currentSection->forwardEdges[inititalSide].targetId = UNINIT_STATE_32;

    if(checkAndDelete(currentSection, forwardEdgeSectionId) == false)
    {
        if(currentSection->sourceId != UNINIT_STATE_32)
        {
            newEdge.targetId = currentSection->sourceId;
            m_metaData.neighors[currentSection->sourceSide].outgoBuffer->addData(&newEdge);
        }
    }
}

/**
 * @brief EdgeCluster::processStatusEdge process status
 * @param forwardEdgeSectionId forward-edge-section in the current cluster
 * @param updateValue incoming diff-value
 * @param updateType delete connection to the new edge
 * @param inititalSide side where the status-value comes in
 */
inline void
EdgeCluster::processUpdateEdge(const uint32_t forwardEdgeSectionId,
                               float updateValue,
                               const uint8_t updateType,
                               const uint8_t inititalSide)
{
    KyoChanForwardEdgeSection* currentSection = &(getForwardEdgeSectionBlock()[forwardEdgeSectionId]);

    if(currentSection->status == ACTIVE_SECTION)
    {
        switch (updateType)
        {
            case KyoChanUpdateEdgeContainer::SET_TYPE: {
                processUpdateSetEdge(currentSection, updateValue, inititalSide);
                break;
            }
            case KyoChanUpdateEdgeContainer::SUB_TYPE: {
                processUpdateSubEdge(currentSection, updateValue, inititalSide);
                break;
            }
            case KyoChanUpdateEdgeContainer::DELETE_TYPE: {
                processUpdateDeleteEdge(currentSection, forwardEdgeSectionId, inititalSide);
                break;
            }
            default:
                break;
        }
    }
}

/**
 * @brief EdgeCluster::processAxon process axon
 * @param targetId section-target-id at the end of the axon
 * @param path rest of the path of the axon
 * @param weight weight of the axon
 */
inline void
EdgeCluster::processAxon(const uint32_t targetId,
                         const uint64_t path,
                         const float weight)
{
    if(path != 0)
    {
        // forward axon the the next in the path
        KyoChanAxonEdgeContainer newEdge;
        newEdge.targetClusterPath = path / 32;
        newEdge.weight = weight * m_globalValue.globalGlia;
        newEdge.targetAxonId = targetId;
        m_metaData.neighors[path % 32].outgoBuffer->addData(&newEdge);
    }
    else
    {
        m_processingData.numberOfActiveAxons++;
        m_processingData.averagetAxonPotential += weight;

        // if target cluster reached, update the state of the target-axon with the edge
        processEdgeForwardSection(targetId, weight);
    }
}

/**
 * @brief EdgeCluster::processLerningEdge process learning-edge to create a new forward-edge
 * @param sourceEdgeSectionId id of the source-edge-section within the last cluster
 * @param weight weight of the new edge
 * @param initSide side of the incoming message
 */
inline void
EdgeCluster::processLerningEdge(const uint32_t sourceEdgeSectionId,
                                const float weight,
                                const uint8_t initSide)
{
    const uint32_t targetEdgeSectionId = addEmptyForwardEdgeSection(initSide,
                                                                    sourceEdgeSectionId);

    if(targetEdgeSectionId != UNINIT_STATE_32)
    {
        // create reply-message
        KyoChanLearningEdgeReplyContainer reply;
        reply.sourceEdgeSectionId = sourceEdgeSectionId;
        reply.targetEdgeSectionId = targetEdgeSectionId;
        m_metaData.neighors[initSide].outgoBuffer->addData(&reply);

        processEdgeForwardSection(targetEdgeSectionId, weight);
    }
}

/**
 * @brief EdgeCluster::processPendingEdge process pending-edge in the cycle after the learning-edge
 * @param sourceId source-section-id of the pending-edge
 * @param sourceSide side of the incoming pending-edge
 * @param weight weight of the pending-edge
 */
inline void
EdgeCluster::processPendingEdge(const uint32_t sourceId,
                                const uint8_t sourceSide,
                                const float weight)
{
    KyoChanForwardEdgeSection* forwardEnd = &(getForwardEdgeSectionBlock()[0]);
    const uint32_t numberOfForwardEdgeSections = m_metaData.numberOfDynamicItems;
    KyoChanForwardEdgeSection* forwardStart = &(getForwardEdgeSectionBlock()[numberOfForwardEdgeSections - 1]);

    // beginn wigh the last forward-edge-section
    uint32_t forwardEdgeSectionId = numberOfForwardEdgeSections - 1;

    // search for the forward-edge-section with the same source-id
    // go backwards through the array, because the target-sections is nearly the end of the array
    for(KyoChanForwardEdgeSection* forwardEdgeSection = forwardStart;
        forwardEdgeSection >= forwardEnd;
        forwardEdgeSection--)
    {
        // if found, then process the pending-edge als normal forward-edge
        if(forwardEdgeSection->status == ACTIVE_SECTION
                && sourceId == forwardEdgeSection->sourceId
                && sourceSide == forwardEdgeSection->sourceSide)
        {
            processEdgeForwardSection(forwardEdgeSectionId, weight);
        }
        forwardEdgeSectionId--;
    }
}

/**
 * @brief EdgeCluster::learningForwardEdgeSection create new edges for the current section
 * @param currentSection pointer to the current section
 * @param forwardEdgeSectionId the id of the current section
 * @param weight weight with have to be consumed from the updated edges
 */
inline void
EdgeCluster::learningForwardEdgeSection(KyoChanForwardEdgeSection *currentSection,
                                        const uint32_t forwardEdgeSectionId,
                                        const float weight)
{
    if(weight >= 0.5f || currentSection->status == ACTIVE_SECTION)
    {
        if(m_metaData.neighors[16].targetClusterId != UNINIT_STATE_32)
        {
            currentSection->forwardEdges[16].weight += weight;
            currentSection->forwardEdges[16].targetId = randomValue(m_fakeRandVal) % 0xFFFF;
        }
        else
        {
            for(uint8_t side = 0; side < 16; side++)
            {
                const float currentSideWeight = weight * m_weightMap[side];

                // set a border to avoid too many new edges
                if(m_metaData.neighors[side].targetClusterId == UNINIT_STATE_32
                        || m_weightMap[side] <= NEW_FORWARD_EDGE_BORDER
                        || currentSideWeight <= 1.0f) {
                    continue;
                }

                // PoC
                if(m_metaData.neighors[side].targetClusterId == 14
                        || m_metaData.neighors[side].targetClusterId == 18
                        || m_metaData.neighors[side].targetClusterId == 22) {
                    continue;
                }

                // cluster-external lerning
                if(currentSection->forwardEdges[side].targetId == UNINIT_STATE_32
                        && currentSection->forwardEdges[side].weight == 0.0f)
                {
                    // send new learning-edge
                    KyoChanLearingEdgeContainer newEdge;
                    newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
                    newEdge.weight = currentSideWeight;
                    m_metaData.neighors[side].outgoBuffer->addData(&newEdge);
                }

                currentSection->forwardEdges[side].weight += currentSideWeight;
            }
        }
    }
}

/**
 * @brief EdgeCluster::processEdgeForwardSection
 * @param forwardEdgeSectionId target edge-section of the incoming edge
 * @param weight incoming edge-weight
 */
inline void
EdgeCluster::processEdgeForwardSection(const uint32_t forwardEdgeSectionId,
                                       const float weight)
{
    KyoChanForwardEdgeSection* currentSection = &(getForwardEdgeSectionBlock()[forwardEdgeSectionId]);
    if(currentSection->status != ACTIVE_SECTION) {
        return;
    }

    // process learning, if the incoming weight is too big
    const float totalWeight = currentSection->getTotalWeight();
    float ratio = weight / totalWeight;
    if(ratio > 1.0f)
    {
        learningForwardEdgeSection(currentSection,
                                   forwardEdgeSectionId,
                                   weight - totalWeight);
        ratio = 1.0f;

        if(checkAndDelete(currentSection, forwardEdgeSectionId)) {
            return;
        }
    }

    // normal processing
    uint8_t sideCounter = 2; // to skip side number 0 and 1
    KyoChanForwardEdge* forwardStart = currentSection->forwardEdges + sideCounter;
    KyoChanForwardEdge* forwardEnd = currentSection->forwardEdges + 17;

    // iterate over all forward-edges in the current section
    for(KyoChanForwardEdge* forwardEdge = forwardStart;
        forwardEdge < forwardEnd;
        forwardEdge++)
    {
        const KyoChanForwardEdge tempForwardEdge = *forwardEdge;

        if(forwardEdge->weight <= 0.0f) {
            sideCounter++;
            continue;
        }

        if(sideCounter != 16)
        {
            if(forwardEdge->targetId != UNINIT_STATE_32)
            {
                // normal external edge
                KyoChanForwardEdgeContainer newEdge;
                newEdge.targetEdgeSectionId = tempForwardEdge.targetId;
                newEdge.weight = tempForwardEdge.weight * ratio;
                m_metaData.neighors[sideCounter].outgoBuffer->addData(&newEdge);
            }
            else
            {
                // send pendinge-edge if the learning-step is not finished
                KyoChanPendingEdgeContainer newEdge;
                newEdge.weight = tempForwardEdge.weight * ratio;
                newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
                newEdge.sourceSide = 16 - sideCounter;
                m_metaData.neighors[sideCounter].outgoBuffer->addData(&newEdge);
            }
        }
        else
        {
            // create direct-edge for outging messages
            KyoChanDirectEdgeContainer newEdge;
            newEdge.targetNodeId = static_cast<uint16_t>(tempForwardEdge.targetId);
            newEdge.weight = tempForwardEdge.weight * ratio;
            m_metaData.neighors[sideCounter].outgoBuffer->addData(&newEdge);
        }

        sideCounter++;
    }
}

}
