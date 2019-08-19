/**
 *  @file    message_processing_methods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef MESSAGE_PROCESSING_METHODS_H
#define MESSAGE_PROCESSING_METHODS_H

#include <common.h>
#include <kyouko_network.h>

#include <core/messaging/message_buffer/outgoing_buffer.h>
#include <core/messaging/message_objects/content_container.h>

#include <core/bricks/brick_objects/brick.h>
#include <core/bricks/brick_methods/buffer_control_methods.h>

namespace KyoukoMind
{

template <typename T>
inline bool
sendData(Brick* brick,
         const uint8_t side,
         const T data,
         const bool force = false)
{
    if(brick->neighbors[side].inUse == 0) {
        return false;
    }

    OutgoingBuffer* outBuffer = &brick->neighbors[side].outgoBuffer;
    DataMessage* message = outBuffer->message;
    assert(message->type != UNDEFINED_MESSAGE);
    memcpy(&message->data[message->size], &data, sizeof(T));
    message->size += sizeof(T);

    if(message->size > 460 || force)
    {
        const uint64_t current = message->currentPosition;
        outBuffer->message = KyoukoNetwork::m_messageBuffer->reserveBuffer(current);
        outBuffer->initMessage();
    }
    return true;
}

/**
 * @brief NodeBrick::createNewEdge
 * @param currentSection
 * @param edgeSectionId
 */
inline void
createNewSynapse(Brick* brick,
                 SynapseSection* currentSection)
{
    Synapse newSynapse;
    newSynapse.targetNodeId = static_cast<uint16_t>(rand()) % NUMBER_OF_NODES_PER_BRICK;
    newSynapse.memorize = brick->globalValues.globalMemorizingOffset;
    newSynapse.somaDistance = static_cast<uint8_t>(rand() % (MAX_SOMA_DISTANCE - 1)) + 1;
    currentSection->addSynapse(newSynapse);
}

/**
 * @brief EdgeBrick::checkAndDelete
 * @param currentSection
 * @param forwardEdgeSectionId
 */
inline bool
checkAndDelete(Brick* brick,
               EdgeSection *currentSection,
               const uint32_t forwardEdgeSectionId)
{
    if(currentSection->sourceId != UNINIT_STATE_32
            && currentSection->getActiveEdges() == 0)
    {
        UpdateEdgeContainer newEdge;
        newEdge.updateType = UpdateEdgeContainer::DELETE_TYPE;

        if(currentSection->sourceId != UNINIT_STATE_32)
        {
            newEdge.targetId = currentSection->sourceId;
            sendData(brick, currentSection->sourceSide, newEdge);
        }

        deleteDynamicItem(brick, EDGE_DATA, forwardEdgeSectionId);

        return true;
    }
    return false;
}

/**
 * learing-process of the specific synapse-section
 *
 * @param currentSection synapse-section with should learn the new value
 * @param weight weight-difference to learn
 */
inline void
learningSynapseSection(Brick* brick,
                       SynapseSection* currentSection,
                       float weight)
{
    while(weight > 0.0f)
    {
        // calculate new value
        float value = weight;
        if(value > MINIMUM_NEW_EDGE_BODER * 2) {
            value /= 2.0f;
        }
        weight -= value;

        // choose synapse
        uint32_t choosePosition = static_cast<uint32_t>(rand()) % (currentSection->numberOfSynapses + 1);

        // create new synapse if necessary
        if(choosePosition == currentSection->numberOfSynapses)
        {
            createNewSynapse(brick, currentSection);
        }
        // synapses, which are fully memorized, are not allowed to be overwritten!
        else if(currentSection->synapses[choosePosition].memorize >= 0.99f)
        {
            choosePosition = currentSection->numberOfSynapses;
            createNewSynapse(brick, currentSection);
        }

        Synapse* synapse = &currentSection->synapses[choosePosition];
        Node* nodeBuffer = (Node*)brick->dataConnections[NODE_DATA].buffer.data;
        Node* node = &nodeBuffer[synapse->targetNodeId];

        const uint8_t tooHeight = nodeBuffer[synapse->targetNodeId].tooHeight;

        float newVal = 0.0f;
        if(brick->isOutputBrick == 0)
        {
            newVal = brick->globalValues.globalLearningOffset * value * ((tooHeight * -2) + 1);
            // make sure it is not too height
            if(node->currentState + newVal > 1.1f * node->border)
            {
                const float diff = (node->currentState + newVal) - (1.1f * node->border);
                newVal -= diff;
            }
        }
        else
        {
            newVal = brick->learningOverride * value;
        }

        currentSection->synapses[choosePosition].weight += newVal;
    }
}

 /**
  * @brief NodeBrick::checkEdge
  * @param currentSection
  * @param weight
  * @return
  */
 inline float
 checkSynapse(Brick* brick,
              SynapseSection* currentSection,
              const float weight)
 {
     const float totalWeight = currentSection->getTotalWeight();
     const float ratio = weight / totalWeight;

     if(ratio > 1.0f)
     {
         if(weight - totalWeight >= NEW_SYNAPSE_BORDER
                 && brick->globalValues.globalLearningOffset > 0.01f)
         {
             learningSynapseSection(brick, currentSection, weight - totalWeight);
         }
         return 1.0f;
     }
     return ratio;
 }

 /**
 * process of a specific edge-section of a brick
 *
 * @param edgeSectionId id of the edge-section within the current brick
 * @param weight incoming weight-value
 */
inline void
processSynapseSection(Brick* brick,
                      const uint32_t synapseSectionId,
                      const float inputWeight)
{
    DataConnection* connection = &brick->dataConnections[SYNAPSE_DATA];
    if(connection->inUse == 0) {
        return;
    }
    SynapseSection* currentSection = &getSynapseSectionBlock(connection)[synapseSectionId];

    // preCheck
    if(inputWeight < 0.5f || currentSection->status != ACTIVE_SECTION) {
        return;
    }

    const float ratio = checkSynapse(brick, currentSection, inputWeight);

    Node* nodes = (Node*)brick->dataConnections[NODE_DATA].buffer.data;
    Synapse* end = currentSection->synapses + currentSection->numberOfSynapses;
    for(Synapse* synapse = currentSection->synapses;
        synapse < end;
        synapse++)
    {
        const Synapse tempSynapse = *synapse;
        nodes[tempSynapse.targetNodeId].currentState += tempSynapse.weight * ratio * ((float)tempSynapse.somaDistance / (float)MAX_SOMA_DISTANCE);
        synapse->inProcess = nodes[tempSynapse.targetNodeId].active;
    }
}

/**
 * @brief EdgeBrick::processUpdateSetEdge
 * @param currentSection
 * @param updateValue
 * @param inititalSide
 */
inline void
processUpdateSetEdge(Brick* brick,
                            EdgeSection *currentSection,
                            const float updateValue,
                            const uint8_t inititalSide)
{
    if(updateValue > 0.0f)
    {
        UpdateEdgeContainer newEdge;
        newEdge.updateValue = updateValue;
        currentSection->edges[inititalSide].weight = updateValue;

        if(currentSection->sourceId != UNINIT_STATE_32)
        {
            newEdge.targetId = currentSection->sourceId;
            sendData(brick, currentSection->sourceSide, newEdge);
        }
    }
}

/**
 * @brief EdgeBrick::processUpdateSubEdge
 * @param currentSection
 * @param updateValue
 * @param inititalSide
 */
inline void
processUpdateSubEdge(Brick* brick,
                            EdgeSection* currentSection,
                            float updateValue,
                            const uint8_t inititalSide)
{
    UpdateEdgeContainer newEdge;
    newEdge.updateValue = updateValue;

    currentSection->edges[inititalSide].weight -= updateValue;

    UpdateEdgeContainer replyEdge;
    replyEdge.updateType = UpdateEdgeContainer::SET_TYPE;
    replyEdge.targetId = currentSection->edges[inititalSide].targetId;
    replyEdge.updateValue = currentSection->edges[inititalSide].weight;

    newEdge.targetId = currentSection->sourceId;
    sendData(brick, currentSection->sourceSide, replyEdge);

    if(currentSection->sourceId != UNINIT_STATE_32)
    {
        newEdge.targetId = currentSection->sourceId;
        sendData(brick, currentSection->sourceSide, newEdge);
    }
}

/**
 * @brief EdgeBrick::processUpdateDeleteEdge
 * @param currentSection
 * @param inititalSide
 */
inline void
processUpdateDeleteEdge(Brick* brick,
                               EdgeSection *currentSection,
                               const uint32_t forwardEdgeSectionId,
                               const uint8_t inititalSide)
{
    UpdateEdgeContainer newEdge;
    newEdge.updateValue = currentSection->edges[inititalSide].weight;

    if(inititalSide < 25)
    {
        currentSection->edges[inititalSide].weight = 0.0f;
        currentSection->edges[inititalSide].targetId = UNINIT_STATE_32;
    }

    if(checkAndDelete(brick, currentSection, forwardEdgeSectionId) == false)
    {
        if(currentSection->sourceId != UNINIT_STATE_32)
        {
            newEdge.targetId = currentSection->sourceId;
            sendData(brick, currentSection->sourceSide, newEdge);
        }
    }
}

/**
 * process status
 *
 * @param forwardEdgeSectionId forward-edge-section in the current brick
 * @param updateValue incoming diff-value
 * @param updateType delete connection to the new edge
 * @param inititalSide side where the status-value comes in
 */
inline void
processUpdateEdge(Brick* brick,
                         const uint32_t forwardEdgeSectionId,
                         float updateValue,
                         const uint8_t updateType,
                         const uint8_t inititalSide)
{
    DataConnection* connection = &brick->dataConnections[EDGE_DATA];
    EdgeSection* currentSection = &getEdgeBlock(connection)[forwardEdgeSectionId];

    if(currentSection->status == ACTIVE_SECTION)
    {
        switch (updateType)
        {
            case UpdateEdgeContainer::SET_TYPE:
            {
                processUpdateSetEdge(brick, currentSection, updateValue, inititalSide);
                break;
            }
            case UpdateEdgeContainer::SUB_TYPE:
            {
                processUpdateSubEdge(brick, currentSection, updateValue, inititalSide);
                break;
            }
            case UpdateEdgeContainer::DELETE_TYPE:
            {
                processUpdateDeleteEdge(brick, currentSection, forwardEdgeSectionId, inititalSide);
                break;
            }
            default:
                break;
        }
    }
}

/**
 * create new edges for the current section
 *
 * @param currentSection pointer to the current section
 * @param forwardEdgeSectionId the id of the current section
 * @param weight weight with have to be consumed from the updated edges
 */
inline void
learningEdgeSection(Brick* brick,
                           EdgeSection *currentSection,
                           float* weightMap,
                           const uint32_t forwardEdgeSectionId,
                           const float weight)
{
    if(weight >= 0.5f || currentSection->status == ACTIVE_SECTION)
    {
        for(uint8_t side = 0; side < 25; side++)
        {
            const float currentSideWeight = weight * weightMap[side];

            if(side == 24)
            {
                if(brick->isInputBrick
                        || weightMap[side] <= NEW_FORWARD_EDGE_BORDER
                        || currentSideWeight <= 1.0f)
                {
                    continue;
                }

                currentSection->edges[24].weight += currentSideWeight;
                uint32_t edgeSectionId = currentSection->edges[24].targetId;

                if(edgeSectionId == UNINIT_STATE_32)
                {
                    edgeSectionId =  addEmptySynapseSection(brick, forwardEdgeSectionId);
                    assert(edgeSectionId != UNINIT_STATE_32);
                    currentSection->edges[24].targetId = edgeSectionId;
                }

                processSynapseSection(brick,
                                   edgeSectionId,
                                   currentSection->edges[24].weight);
            }
            else
            {
                // set a border to avoid too many new edges
                if(brick->neighbors[side].targetBrickId == UNINIT_STATE_32
                        || weightMap[side] <= NEW_FORWARD_EDGE_BORDER
                        || currentSideWeight <= 1.0f)
                {
                    continue;
                }

                // brick-external lerning
                if(currentSection->edges[side].targetId == UNINIT_STATE_32
                        && currentSection->edges[side].weight == 0.0f)
                {
                    // send new learning-edge
                    LearingEdgeContainer newEdge;
                    newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
                    newEdge.weight = currentSideWeight;
                    sendData(brick, side, newEdge);
                }

                currentSection->edges[side].weight += currentSideWeight;
            }
        }
    }
}

/**
 * processEdgeForwardSection
 *
 * @param forwardEdgeSectionId target edge-section of the incoming edge
 * @param weight incoming edge-weight
 */
inline void
processEdgeForwardSection(Brick* brick,
                          const uint32_t forwardEdgeSectionId,
                          const float weight,
                          float* weightMap)
{
    DataConnection* connection = &brick->dataConnections[EDGE_DATA];
    EdgeSection* currentSection = &getEdgeBlock(connection)[forwardEdgeSectionId];

    if(currentSection->status != ACTIVE_SECTION) {
        return;
    }

    // process learning, if the incoming weight is too big
    const float totalWeight = currentSection->getTotalWeight();
    float ratio = weight / totalWeight;
    if(ratio > 1.0f)
    {
        learningEdgeSection(brick,
                            currentSection,
                            weightMap,
                            forwardEdgeSectionId,
                            weight - totalWeight);
        ratio = 1.0f;

        if(checkAndDelete(brick, currentSection, forwardEdgeSectionId)) {
            return;
        }
    }

    // iterate over all forward-edges in the current section
    for(uint8_t sideCounter = 0; sideCounter < 25; sideCounter++)
    {
        const Edge tempEdge = currentSection->edges[sideCounter];
        if(tempEdge.weight <= 0.0f) {
            continue;
        }

        if(sideCounter == 24)
        {
            assert(tempEdge.targetId != UNINIT_STATE_32);
            processSynapseSection(brick, tempEdge.targetId, tempEdge.weight);
        }
        else
        {
            if(tempEdge.targetId != UNINIT_STATE_32)
            {
                // normal external edge
                EdgeContainer newEdge;
                newEdge.targetEdgeSectionId = tempEdge.targetId;
                newEdge.weight = tempEdge.weight * ratio;
                sendData(brick, sideCounter, newEdge);
            }
            else
            {
                // send pendinge-edge if the learning-step is not finished
                PendingEdgeContainer newEdge;
                newEdge.weight = tempEdge.weight * ratio;
                newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
                newEdge.sourceSide = 23 - sideCounter;
                sendData(brick, sideCounter, newEdge);
            }
        }
    }
}

/**
 * process axon
 *
 * @param targetId section-target-id at the end of the axon
 * @param path rest of the path of the axon
 * @param weight weight of the axon
 */
inline void
processAxon(Brick* brick,
            const uint32_t targetId,
            const uint64_t path,
            const float weight,
            float* weightMap)
{
    if(path != 0)
    {
        // forward axon the the next in the path
        AxonEdgeContainer newEdge;
        newEdge.targetBrickPath = path / 32;
        newEdge.weight = weight * brick->globalValues.globalGlia;
        newEdge.targetAxonId = targetId;
        const uint8_t side = path % 32;
        sendData(brick, side, newEdge);
    }
    else
    {
        // if target brick reached, update the state of the target-axon with the edge
        processEdgeForwardSection(brick, targetId, weight, weightMap);
    }
}

/**
 * process learning-edge to create a new forward-edge
 *
 * @param sourceEdgeSectionId id of the source-edge-section within the last brick
 * @param weight weight of the new edge
 * @param initSide side of the incoming message
 */
inline void
processLerningEdge(Brick* brick,
                          const uint32_t sourceEdgeSectionId,
                          const float weight,
                          const uint8_t initSide,
                          float* weightMap)
{
    const uint32_t targetEdgeSectionId = addEmptyEdgeSection(brick,
                                                                    initSide,
                                                                    sourceEdgeSectionId);

    if(targetEdgeSectionId != UNINIT_STATE_32)
    {
        // create reply-message
        LearningEdgeReplyContainer reply;
        reply.sourceEdgeSectionId = sourceEdgeSectionId;
        reply.targetEdgeSectionId = targetEdgeSectionId;
        sendData(brick, initSide, reply);

        processEdgeForwardSection(brick, targetEdgeSectionId, weight, weightMap);
    }
}

/**
 * process pending-edge in the cycle after the learning-edge
 *
 * @param sourceId source-section-id of the pending-edge
 * @param sourceSide side of the incoming pending-edge
 * @param weight weight of the pending-edge
 */
inline void
processPendingEdge(Brick* brick,
                          const uint32_t sourceId,
                          const uint8_t sourceSide,
                          const float weight,
                          float *weightMap)
{
    DataConnection* connection = &brick->dataConnections[EDGE_DATA];

    const uint32_t numberOfEdgeSections = connection->numberOfItems;
    EdgeSection* forwardEnd = getEdgeBlock(connection);
    EdgeSection* forwardStart = &forwardEnd[numberOfEdgeSections - 1];

    // beginn wigh the last forward-edge-section
    uint32_t forwardEdgeSectionId = numberOfEdgeSections - 1;

    // search for the forward-edge-section with the same source-id
    // go backwards through the array, because the target-sections is nearly the end of the array
    for(EdgeSection* forwardEdgeSection = forwardStart;
        forwardEdgeSection >= forwardEnd;
        forwardEdgeSection--)
    {
        // if found, then process the pending-edge als normal forward-edge
        if(forwardEdgeSection->status == ACTIVE_SECTION
                && sourceId == forwardEdgeSection->sourceId
                && sourceSide == forwardEdgeSection->sourceSide)
        {
            processEdgeForwardSection(brick, forwardEdgeSectionId, weight, weightMap);
        }
        forwardEdgeSectionId--;
    }
}

} // namespace KyoukoMind

#endif // MESSAGE_PROCESSING_METHODS_H
