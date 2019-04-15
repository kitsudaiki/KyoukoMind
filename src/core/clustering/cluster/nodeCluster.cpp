/**
 *  @file    nodeCluster.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/clustering/cluster/nodeCluster.h>

#include <core/messaging/incomingMessageBuffer.h>
#include <core/messaging/outgoingMessageBuffer.h>

#include <kyoChanNetwork.h>
#include <commonDataBuffer.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>

#include <kyoChanNetwork.h>

namespace KyoukoMind
{

/**
 * @brief NodeCluster::NodeCluster constructor for node-cluster-creation
 * @param clusterId id for the new cluster
 * @param directoryPath path to the directory, where the file of the cluster should be saved
 * @param numberOfNodes number of nodes of the new node-cluster
 */
NodeCluster::NodeCluster(const ClusterID clusterId,
                         const uint32_t x,
                         const uint32_t y,
                         const std::string directoryPath,
                         const uint32_t numberOfNodes)
    : Cluster(clusterId,
              x,
              y,
              directoryPath,
              sizeof(KyoChanNode),
              sizeof(KyoChanEdgeSection))
{
    m_metaData.clusterType = NODE_CLUSTER;
    m_metaData.positionOfStaticBlocks = 1;
    initNodeBlocks(numberOfNodes);
}

/**
 * @brief NodeCluster::getNumberOfNodeBlocks get number of node-blocks of the cluster in the buffer
 * @return number of node-blocks
 */
uint32_t
NodeCluster::getNumberOfNodeBlocks() const
{
    return m_metaData.numberOfStaticBlocks;
}

/**
 * @brief NodeCluster::getNumberOfNode get number of nodes of the cluster
 * @return number of nodes
 */
uint32_t
NodeCluster::getNumberOfNodes() const
{
    return m_metaData.numberOfStaticItems;
}

/**
 * @brief NodeCluster::getNodeBlock get pointer to the node-list of the cluster
 * @return pointer to the first block of the node-list
 */
KyoChanNode*
NodeCluster::getNodeBlock()
{
    const uint32_t positionNodeBlock = m_metaData.positionOfStaticBlocks;
    return (KyoChanNode*)m_clusterDataBuffer->getBlock(positionNodeBlock);
}

/**
 * @brief NodeCluster::initNodeBlocks initialize the node-list of the cluster
 * @param numberOfNodes number of new empty nodes
 * @return false if nodes are already initialized, esle true
 */
bool
NodeCluster::initNodeBlocks(uint32_t numberOfNodes)
{
    // prechecks
    if(m_metaData.numberOfStaticItems != 0) {
        return false;
    }

    if(numberOfNodes == 0) {
        numberOfNodes = NUMBER_OF_NODES_PER_CLUSTER;
    }

    if(initStaticBlocks(numberOfNodes) == false) {
        return false;
    }

    // fill array with empty nodes
    KyoChanNode* array = getNodeBlock();
    for(uint16_t i = 0; i < numberOfNodes; i++)
    {
        KyoChanNode tempNode;
        tempNode.border = (randomValue() % (MAXIMUM_NODE_BODER - MINIMUM_NODE_BODER)) + MINIMUM_NODE_BODER;
        array[i] = tempNode;
    }

    return true;
}

/**
 * @brief NodeCluster::getNumberOfEdgeSectionBlocks get number of edge-section-blocks of the cluster in the buffer
 * @return number of edge-section-blocks
 */
uint32_t
NodeCluster::getNumberOfEdgeSectionBlocks() const
{
    return m_metaData.numberOfDynamicBlocks;
}

/**
 * @brief NodeCluster::getNumberOfEdgeSections get number of edge-sections in the cluster
 * @return number of edge-sections
 */
uint32_t
NodeCluster::getNumberOfEdgeSections() const
{
    return m_metaData.numberOfDynamicItems;
}

/**
 * @brief NodeCluster::getEdgeSectionBlock get the edge-section-block
 * @return pointer to the beginning of the edge-section-block
 */
KyoChanEdgeSection*
NodeCluster::getEdgeSectionBlock()
{
    uint32_t positionEdgeBlock = m_metaData.positionOfDynamicBlocks;
    return (KyoChanEdgeSection*)m_clusterDataBuffer->getBlock(positionEdgeBlock);
}

/**
 * @brief NodeCluster::initEdgeSectionBlocks init the edge-sections of thecluster
 * @param numberOfEdgeSections number of edge-sections which should be initialized
 * @return false, if already initialized, else true
 */
bool
NodeCluster::initEdgeSectionBlocks(const uint32_t numberOfEdgeSections)
{
    initDynamicBlocks(numberOfEdgeSections);

    // fill array with empty edgesections
    KyoChanEdgeSection* array = getEdgeSectionBlock();
    for(uint32_t i = 0; i < numberOfEdgeSections; i++)
    {
        KyoChanEdgeSection newSection;
        newSection.sourceId = i;
        array[i] = newSection;
    }

    return true;
}

/**
 * @brief NodeCluster::addEdge add an existing edge to a specifig edge-sections
 * @param edgeSectionId id of the edge-section for the new edge
 * @param newEdge new edge, which should be added
 * @return false, if edgeSectionId is too big, else true
 */
bool
NodeCluster::addEdge(const uint32_t edgeSectionId,
                     const KyoChanEdge &newEdge)
{
    // check if id is valid
    if(edgeSectionId >= m_metaData.numberOfDynamicItems) {
        return false;
    }

    // get section and add the new edge
    KyoChanEdgeSection* edgeSection = &getEdgeSectionBlock()[edgeSectionId];
    return edgeSection->addEdge(newEdge);
}

/**
 * @brief NodeCluster::getSummedValue
 * @return
 */
float NodeCluster::getSummedValue()
{
    float sum = 0.0f;

    // process nodes
    KyoChanNode* start = getNodeBlock();
    KyoChanNode* end = start + getNumberOfNodes();

    // iterate over all nodes in the cluster
    for(KyoChanNode* node = start;
        node < end;
        node++)
    {
        sum += node->currentState;
    }
    return sum;
}

/**
 * @brief NodeCluster::setAsOutput
 */
void NodeCluster::setAsOutput()
{
    m_metaData.outputCluster = 1;

    // process nodes
    KyoChanNode* start = getNodeBlock();
    KyoChanNode* end = start + getNumberOfNodes();

    // iterate over all nodes in the cluster
    for(KyoChanNode* node = start;
        node < end;
        node++)
    {
        node->border = 1000.0f;
    }

    updateMetaData();
}

/**
 * @brief NodeCluster::addEmptyEdgeSection add a new empfy edge-section
 * @return id of the new section, or SPECIAL_STATE if memory-allocation failed
 * @param sourceId id of the source forward-edge-section
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
uint32_t
NodeCluster::addEmptyEdgeSection(const uint8_t sourceSide,
                                 const uint32_t sourceId)
{
    const uint32_t position = reserveDynamicItem();

    if(position != UNINIT_STATE_32) {
        // add new edge-forward-section
        KyoChanEdgeSection newSection;
        newSection.sourceId = sourceId;
        newSection.sourceSide = sourceSide;
        getEdgeSectionBlock()[position] = newSection;
    }

    return position;
}

/**
 * @brief NodeClusterProcessing::processMessagesNodeCluster processing of all incoming messages in a cluster
 * @return false if a message-type does not exist, else true
 */
bool
NodeCluster::processNodeCluster()
{
    bool result = true;
    initCycle();

    // process normal communication
    for(uint8_t sidePos = 0; sidePos < m_numberOfIncomingMessages; sidePos++)
    {
        const uint8_t side = m_incomingMessages[sidePos].second;

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
                    if(edge->targetId == UNINIT_STATE_32) {
                        continue;
                    }
                    processUpdateEdge(edge->targetId, edge->updateValue, edge->updateType);
                    break;
                }
                // ------------------------------------------------------------------------------------------
                case LEARNING_EDGE_CONTAINER:
                {
                    KyoChanLearingEdgeContainer* edge = (KyoChanLearingEdgeContainer*)data;
                    data += sizeof(KyoChanLearingEdgeContainer);
                    if(edge->sourceEdgeSectionId == UNINIT_STATE_32) {
                        continue;
                    }
                    processLerningEdge(edge->sourceEdgeSectionId, edge->weight, side);
                    break;
                }
                // ------------------------------------------------------------------------------------------
                case PENDING_EDGE_CONTAINER:
                {
                    KyoChanPendingEdgeContainer* edge = (KyoChanPendingEdgeContainer*)data;
                    data += sizeof(KyoChanPendingEdgeContainer);
                    if(edge->sourceEdgeSectionId == UNINIT_STATE_32) {
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
                    if(edge->targetEdgeSectionId == UNINIT_STATE_32) {
                        continue;
                    }
                    processEdgeSection(edge->targetEdgeSectionId, edge->weight);
                    break;
                }
                // ------------------------------------------------------------------------------------------
                case DIRECT_EDGE_CONTAINER:
                {
                    KyoChanDirectEdgeContainer* edge = (KyoChanDirectEdgeContainer*)data;
                    data += sizeof(KyoChanDirectEdgeContainer);
                    if(edge->targetNodeId == UNINIT_STATE_16) {
                        continue;
                    }
                    getNodeBlock()[edge->targetNodeId].currentState += edge->weight;
                    break;
                }
                // ------------------------------------------------------------------------------------------
                default:
                    result = false;
            }
        }
    }

    postLearning();

    // memorizing
    memorizeEdges();
    /*if(m_memorizeCounter % MEMORIZE_INTERVAL == 0 && result == true)
    {
        memorizeEdges();
        m_memorizeCounter = 0;
    }
    m_memorizeCounter++;*/

    finishCycle();

    if(m_metaData.outputCluster == 1)
    {
        float sum = getSummedValue();
        KyoukoNetwork::m_mindClient->sendData(m_metaData.clusterId, sum);
    }
    return result;
}

/**
 * @brief NodeCluster::processNodes processing of the nodes of a specific node-cluster
 * @return number of active nodes in this cluster
 */
uint16_t
NodeCluster::processNodes()
{
    if(m_metaData.outputCluster == 1) {
        // process nodes
        KyoChanNode* start = getNodeBlock();
        KyoChanNode* end = start + getNumberOfNodes();

        // iterate over all nodes in the cluster
        for(KyoChanNode* node = start;
            node < end;
            node++)
        {
            node->currentState /= NODE_COOLDOWN;
        }
        return 0;
    }

    uint16_t numberOfActiveNodes = 0;
    float totalPotential = 0.0f;

    // process nodes
    KyoChanNode* start = getNodeBlock();
    KyoChanNode* end = start + getNumberOfNodes();

    // iterate over all nodes in the cluster
    for(KyoChanNode* node = start;
        node < end;
        node++)
    {
        // limit the node-state to 255
        if(node->currentState > 255.0f) {
            node->currentState = 255.0f;
        }

        // init
        const KyoChanNode tempNode = *node;

        // check if active
        if(tempNode.border <= tempNode.currentState
                && tempNode.refractionTime == 0)
        {
            node->potential = ACTION_POTENTIAL;
            numberOfActiveNodes++;
            node->active = 1;
            node->refractionTime = REFRACTION_TIME;
        }
        else if(tempNode.refractionTime == 0) {
            node->active = 0;
        }

        // forward current potential
        if(node->active == 1)
        {
            KyoChanAxonEdgeContainer edge;
            edge.targetClusterPath = tempNode.targetClusterPath / 32;
            edge.targetAxonId = tempNode.targetAxonId;
            edge.weight = node->potential * PROCESSING_MULTIPLICATOR;
            m_metaData.neighors[tempNode.targetClusterPath % 32].outgoBuffer->addData(&edge);
        }

        // post-steps
        if(node->refractionTime != 0) {
            node->refractionTime--;
        }
        if(node->currentState < 0.0f) {
            node->currentState = 0.0f;
        }

        if(node->currentState > 1.2f * node->border) {
            node->tooHeight = 1;
        } else {
            node->tooHeight = 0;
        }

        // make cooldown in the node
        totalPotential += node->potential;
        node->potential /= NODE_COOLDOWN;
        node->currentState /= NODE_COOLDOWN;
    }

    m_monitoringProcessingData.numberOfActiveAxons += numberOfActiveNodes;
    m_monitoringProcessingData.averagetAxonPotential += totalPotential;

    m_neighborInfo.localLearing += (float) numberOfActiveNodes / (float)m_metaData.numberOfStaticItems;
    return numberOfActiveNodes;
}

/**
 * @brief NodeCluster::memorizeEdges
 */
void
NodeCluster::postLearning()
{
    KyoChanEdgeSection* sectionStart = getEdgeSectionBlock();
    KyoChanEdgeSection* sectionEnd = sectionStart + getNumberOfEdgeSections();

    // iterate over all edge-sections
    for(KyoChanEdgeSection* section = sectionStart;
        section < sectionEnd;
        section++)
    {
        // update values based on the memorizing-value
        KyoChanEdge* end = section->edges + section->numberOfEdges;
        for(KyoChanEdge* edge = section->edges;
            edge < end;
            edge++)
        {

            // normal learn
            /*if(edge->inProcess == 1
                    && m_globalValue.globalLearningTemp != 0.0f
                    && edge->targetNodeId != NUMBER_OF_NODES_PER_CLUSTER - 1)
            {
                const float diff = 1.0f - edge->memorize;
                edge->weight += (diff * m_globalValue.globalLearningTemp);
            }*/

            // mem
            if(m_globalValue.globalMemorizingTemp != 0.0f)
            {
                edge->memorize = m_globalValue.globalMemorizingTemp;
                if(edge->memorize > 1.0f) {
                    edge->memorize = 1.0f;
                }
            }
        }
    }
}

/**
 * @brief NodeCluster::memorizeEdges process memorizing
 */
void
NodeCluster::memorizeEdges()
{
    OutgoingMessageBuffer* outgoBuffer = getOutgoingMessageBuffer(8);
    KyoChanEdgeSection* sectionStart = getEdgeSectionBlock();
    KyoChanEdgeSection* sectionEnd = sectionStart + getNumberOfEdgeSections();



    // iterate over all edge-sections
    uint32_t sectionPos = 0;
    for(KyoChanEdgeSection* section = sectionStart;
        section < sectionEnd;
        section++)
    {
        // skip if section is deleted
        if(section->status != ACTIVE_SECTION) {
            sectionPos++;
            continue;
        }

        KyoChanUpdateEdgeContainer newEdge;
        newEdge.targetId = section->sourceId;

        // update values based on the memorizing-value
        KyoChanEdge* end = section->edges + section->numberOfEdges;
        for(KyoChanEdge* edge = section->edges;
            edge < end;
            edge++)
        {
            if(edge->inProcess == 1) { continue; }



            const float newWeight = edge->weight * (1.0f - edge->memorize);
            edge->weight -= newWeight;
        }
        section->makeClean();

        // delete dynamic item if value is too low
        if(section->getTotalWeight() < DELETE_EDGE_BORDER)
        {
            newEdge.updateType = KyoChanUpdateEdgeContainer::DELETE_TYPE;
            outgoBuffer->addData(&newEdge);
            deleteDynamicItem(sectionPos);
        }
        else
        {
            newEdge.updateType = KyoChanUpdateEdgeContainer::SET_TYPE;
            newEdge.updateValue = section->getTotalWeight();
            if(newEdge.updateValue > 0.0f) {
                outgoBuffer->addData(&newEdge);
            }
        }
        sectionPos++;
    }
}

/**
 * @brief NodeCluster::processUpdateSetEdge
 * @param currentSection
 * @param updateValue
 */
void NodeCluster::processUpdateSetEdge(KyoChanEdgeSection *currentSection,
                                       float updateValue)
{
    const float ratio = updateValue / currentSection->getTotalWeight();

    KyoChanEdge* end = currentSection->edges + currentSection->numberOfEdges;
    for(KyoChanEdge* edge = currentSection->edges;
        edge < end;
        edge++)
    {
        edge->weight *= ratio;
    }
}

/**
 * @brief NodeCluster::processUpdateSubEdge
 * @param currentSection
 * @param updateValue
 */
void NodeCluster::processUpdateSubEdge(KyoChanEdgeSection *currentSection,
                                       float updateValue)
{
    const float totalWeight = currentSection->getTotalWeight();
    const float ratio = (totalWeight - updateValue) / totalWeight;

    KyoChanEdge* end = currentSection->edges + currentSection->numberOfEdges;
    for(KyoChanEdge* edge = currentSection->edges;
        edge < end;
        edge++)
    {
        edge->weight *= ratio;
    }
}

/**
 * @brief NodeCluster::processUpdateEdge
 * @param edgeSectionId
 * @param updateValue
 * @param updateType
 */
inline void
NodeCluster::processUpdateEdge(const uint32_t edgeSectionId,
                               const float updateValue,
                               const uint8_t updateType)
{
    KyoChanEdgeSection* currentSection = &(getEdgeSectionBlock()[edgeSectionId]);

    if(currentSection->status == ACTIVE_SECTION)
    {
        switch (updateType)
        {
            case KyoChanUpdateEdgeContainer::SET_TYPE: {
                processUpdateSetEdge(currentSection, updateValue);
                break;
            }
            case KyoChanUpdateEdgeContainer::SUB_TYPE: {
                processUpdateSubEdge(currentSection, updateValue);
                break;
            }
            default:
                break;
        }
    }
}

/**
 * @brief NodeCluster::processLerningEdge process learning
 * @param sourceEdgeSectionId id of the edge-section where the edge comes from
 * @param weight weight of the new edge
 * @param initSide initial side of the edge
 */
inline void
NodeCluster::processLerningEdge(const uint32_t sourceEdgeSectionId,
                                const float weight,
                                const uint8_t initSide)
{
    assert(weight >= 0.0f);

    const uint32_t targetEdgeSectionId = addEmptyEdgeSection(initSide, sourceEdgeSectionId);

    if(targetEdgeSectionId != UNINIT_STATE_32)
    {
        // create reply-message
        KyoChanLearningEdgeReplyContainer reply;
        reply.sourceEdgeSectionId = sourceEdgeSectionId;
        reply.targetEdgeSectionId = targetEdgeSectionId;
        m_metaData.neighors[initSide].outgoBuffer->addData(&reply);

        processEdgeSection(targetEdgeSectionId, weight);
    }
}

/**
 * @brief NodeCluster::processPendingEdge process pending-edge in the cycle after the learning-edge
 * @param sourceId source-section-id of the pending-edge
 * @param sourceSide side of the incoming pending-edge
 * @param weight weight of the pending edge
 */
inline void
NodeCluster::processPendingEdge(const uint32_t sourceId,
                                const uint8_t sourceSide,
                                const float weight)
{
    KyoChanEdgeSection* forwardEnd = &(getEdgeSectionBlock()[0]);
    const uint32_t numberOfEdgeSections = getNumberOfEdgeSections();
    KyoChanEdgeSection* forwardStart = &(getEdgeSectionBlock()[numberOfEdgeSections - 1]);

    uint32_t forwardEdgeSectionId = numberOfEdgeSections - 1;
    for(KyoChanEdgeSection* edgeSection = forwardStart;
        edgeSection >= forwardEnd;
        edgeSection--)
    {
        if(edgeSection->status == ACTIVE_SECTION
                && sourceId == edgeSection->sourceId
                && sourceSide == edgeSection->sourceSide)
        {
            processEdgeSection(forwardEdgeSectionId, weight);
        }

        forwardEdgeSectionId--;
    }
}

/**
 * @brief NodeCluster::learningEdgeSection learing-process of the specific edge-section
 * @param currentSection edge-section with should learn the new value
 * @param weight weight-difference to learn
 */
inline void
NodeCluster::learningEdgeSection(KyoChanEdgeSection* currentSection,
                                 float weight,
                                 const uint32_t edgeSectionId)
{
    while(weight > 0.0f)
    {
        // calculate new value
        float value = weight;
        if(value > MINIMUM_NEW_EDGE_BODER * 2) {
            value /= 2.0f;
        }
        weight -= value;

        // choose edge
        uint32_t choosePosition = static_cast<uint32_t>(rand()) % (currentSection->numberOfEdges + 1);

        // create new edge if necessary
        if(choosePosition == currentSection->numberOfEdges)
        {
            createNewEdge(currentSection, edgeSectionId);
        }
        else if(currentSection->edges[choosePosition].memorize >= 0.99f)
        {
            choosePosition = currentSection->numberOfEdges;
            createNewEdge(currentSection, edgeSectionId);
        }

        KyoChanEdge* edge = &currentSection->edges[choosePosition];
        KyoChanNode* node = &getNodeBlock()[edge->targetNodeId];

        const uint8_t tooHeight = getNodeBlock()[edge->targetNodeId].tooHeight;

        float newVal = 0.0f;
        if(m_metaData.outputCluster == 0)
        {
            newVal = m_globalValue.globalLearningOffset * value * ((tooHeight * -2) + 1);
            // make sure it is not too height
            if(node->currentState + newVal > 1.1f * node->border)
            {
                const float diff = (node->currentState + newVal) - (1.1f * node->border);
                newVal -= diff;
            }
        }
        else
        {
            newVal = m_learningOverride * value;
        }

        currentSection->edges[choosePosition].weight += newVal;
    }
}

/**
 * @brief NodeCluster::createNewEdge
 * @param currentSection
 * @param edgeSectionId
 */
inline void
NodeCluster::createNewEdge(KyoChanEdgeSection *currentSection,
                          const uint32_t edgeSectionId)
{
    KyoChanEdge newEdge;
    newEdge.targetNodeId = static_cast<uint16_t>(rand()) % NUMBER_OF_NODES_PER_CLUSTER;
    newEdge.memorize = m_globalValue.globalMemorizingOffset;
    newEdge.somaDistance = static_cast<uint8_t>(rand() % (MAX_SOMA_DISTANCE - 1)) + 1;

    // poc-stuff
    while(newEdge.targetNodeId == edgeSectionId || newEdge.targetNodeId < 3) {
        newEdge.targetNodeId = static_cast<uint16_t>(rand()) % NUMBER_OF_NODES_PER_CLUSTER;
    }

    currentSection->addEdge(newEdge);
}

 /**
  * @brief NodeCluster::checkEdge
  * @param currentSection
  * @param weight
  * @return
  */
 inline float
 NodeCluster::checkEdge(KyoChanEdgeSection* currentSection,
                        const float weight,
                        const uint32_t edgeSectionId)
 {
     const float totalWeight = currentSection->getTotalWeight();
     const float ratio = weight / totalWeight;

     if(ratio > 1.0f)
     {
         if(weight - totalWeight >= NEW_EDGE_BORDER && m_globalValue.globalLearningOffset > 0.01f) {
             learningEdgeSection(currentSection, weight - totalWeight, edgeSectionId);
         }
         return 1.0f;
     }
     return ratio;
 }

 /**
 * @brief NodeCluster::processEdgeSection process of a specific edge-section of a cluster
 * @param edgeSectionId id of the edge-section within the current cluster
 * @param weight incoming weight-value
 */
inline void
NodeCluster::processEdgeSection(const uint32_t edgeSectionId,
                                const float weight)
{
    KyoChanEdgeSection* currentSection = &(getEdgeSectionBlock()[edgeSectionId]);

    // preCheck
    if(weight < 0.5f || currentSection->status != ACTIVE_SECTION) {
        return;
    }

    const float ratio = checkEdge(currentSection, weight, edgeSectionId);

    KyoChanNode* nodes = getNodeBlock();
    KyoChanEdge* end = currentSection->edges + currentSection->numberOfEdges;
    for(KyoChanEdge* edge = currentSection->edges;
        edge < end;
        edge++)
    {
        const KyoChanEdge tempEdge = *edge;
        nodes[tempEdge.targetNodeId].currentState += tempEdge.weight * ratio * ((float)tempEdge.somaDistance / (float)MAX_SOMA_DISTANCE);
        edge->inProcess = nodes[tempEdge.targetNodeId].active;
    }
}

}
