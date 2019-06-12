/**
 *  @file    brickMethods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef NODEBRICKMETHODS_H
#define NODEBRICKMETHODS_H

#include <common.h>
#include <kyoukoNetwork.h>

#include <core/messaging/messageMarker/outgoingBuffer.h>
#include <core/messaging/messageObjects/contentContainer.h>

#include <core/bricks/brickObjects/brick.h>
#include <core/bricks/brickMethods/bufferControlMethods.h>

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
        outBuffer->message = KyoukoNetwork::m_internalMessageBuffer->reserveBuffer(current);
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
createNewEdge(Brick* brick,
              EdgeSection *currentSection)
{
    Edge newEdge;
    newEdge.targetNodeId = static_cast<uint16_t>(rand()) % NUMBER_OF_NODES_PER_BRICK;
    newEdge.memorize = brick->globalValues.globalMemorizingOffset;
    newEdge.somaDistance = static_cast<uint8_t>(rand() % (MAX_SOMA_DISTANCE - 1)) + 1;
    currentSection->addEdge(newEdge);
}

/**
 * @brief EdgeBrick::checkAndDelete
 * @param currentSection
 * @param forwardEdgeSectionId
 */
inline bool
checkAndDelete(Brick* brick,
               ForwardEdgeSection *currentSection,
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

        deleteDynamicItem(brick, FORWARDEDGE_DATA, forwardEdgeSectionId);

        return true;
    }
    return false;
}

/**
 * learing-process of the specific edge-section
 *
 * @param currentSection edge-section with should learn the new value
 * @param weight weight-difference to learn
 */
inline void
learningEdgeSection(Brick* brick,
                    EdgeSection* currentSection,
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

        // choose edge
        uint32_t choosePosition = static_cast<uint32_t>(rand()) % (currentSection->numberOfEdges + 1);

        // create new edge if necessary
        if(choosePosition == currentSection->numberOfEdges)
        {
            createNewEdge(brick, currentSection);
        }
        // edges, which are fully memorized, are not allowed to be overwritten!
        else if(currentSection->edges[choosePosition].memorize >= 0.99f)
        {
            choosePosition = currentSection->numberOfEdges;
            createNewEdge(brick, currentSection);
        }

        Edge* edge = &currentSection->edges[choosePosition];
        Node* nodeBuffer = (Node*)brick->dataConnections[NODE_DATA].buffer.data;
        Node* node = &nodeBuffer[edge->targetNodeId];

        const uint8_t tooHeight = nodeBuffer[edge->targetNodeId].tooHeight;

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

        currentSection->edges[choosePosition].weight += newVal;
    }
}

 /**
  * @brief NodeBrick::checkEdge
  * @param currentSection
  * @param weight
  * @return
  */
 inline float
 checkEdge(Brick* brick,
           EdgeSection* currentSection,
           const float weight)
 {
     const float totalWeight = currentSection->getTotalWeight();
     const float ratio = weight / totalWeight;

     if(ratio > 1.0f)
     {
         if(weight - totalWeight >= NEW_EDGE_BORDER
                 && brick->globalValues.globalLearningOffset > 0.01f)
         {
             learningEdgeSection(brick, currentSection, weight - totalWeight);
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
processEdgeSection(Brick* brick,
                   const uint32_t edgeSectionId,
                   const float inputWeight)
{
    DataConnection* connection = &brick->dataConnections[EDGE_DATA];
    if(connection->inUse == 0) {
        return;
    }
    EdgeSection* currentSection = &getEdgeSectionBlock(connection)[edgeSectionId];

    // preCheck
    if(inputWeight < 0.5f || currentSection->status != ACTIVE_SECTION) {
        return;
    }

    const float ratio = checkEdge(brick, currentSection, inputWeight);

    Node* nodes = (Node*)brick->dataConnections[NODE_DATA].buffer.data;
    Edge* end = currentSection->edges + currentSection->numberOfEdges;
    for(Edge* edge = currentSection->edges;
        edge < end;
        edge++)
    {
        const Edge tempEdge = *edge;
        nodes[tempEdge.targetNodeId].currentState += tempEdge.weight * ratio * ((float)tempEdge.somaDistance / (float)MAX_SOMA_DISTANCE);
        edge->inProcess = nodes[tempEdge.targetNodeId].active;
    }
}

/**
 * @brief EdgeBrick::processUpdateSetEdge
 * @param currentSection
 * @param updateValue
 * @param inititalSide
 */
inline void
processUpdateSetForwardEdge(Brick* brick,
                            ForwardEdgeSection *currentSection,
                            const float updateValue,
                            const uint8_t inititalSide)
{
    if(updateValue > 0.0f)
    {
        UpdateEdgeContainer newEdge;
        newEdge.updateValue = updateValue;
        currentSection->forwardEdges[inititalSide].weight = updateValue;

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
processUpdateSubForwardEdge(Brick* brick,
                            ForwardEdgeSection* currentSection,
                            float updateValue,
                            const uint8_t inititalSide)
{
    UpdateEdgeContainer newEdge;
    newEdge.updateValue = updateValue;

    currentSection->forwardEdges[inititalSide].weight -= updateValue;

    UpdateEdgeContainer replyEdge;
    replyEdge.updateType = UpdateEdgeContainer::SET_TYPE;
    replyEdge.targetId = currentSection->forwardEdges[inititalSide].targetId;
    replyEdge.updateValue = currentSection->forwardEdges[inititalSide].weight;

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
processUpdateDeleteForwardEdge(Brick* brick,
                               ForwardEdgeSection *currentSection,
                               const uint32_t forwardEdgeSectionId,
                               const uint8_t inititalSide)
{
    UpdateEdgeContainer newEdge;
    newEdge.updateValue = currentSection->forwardEdges[inititalSide].weight;

    if(inititalSide < 25)
    {
        currentSection->forwardEdges[inititalSide].weight = 0.0f;
        currentSection->forwardEdges[inititalSide].targetId = UNINIT_STATE_32;
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
processUpdateForwardEdge(Brick* brick,
                         const uint32_t forwardEdgeSectionId,
                         float updateValue,
                         const uint8_t updateType,
                         const uint8_t inititalSide)
{
    DataConnection* connection = &brick->dataConnections[FORWARDEDGE_DATA];
    ForwardEdgeSection* currentSection = &getForwardEdgeBlock(connection)[forwardEdgeSectionId];

    if(currentSection->status == ACTIVE_SECTION)
    {
        switch (updateType)
        {
            case UpdateEdgeContainer::SET_TYPE:
            {
                processUpdateSetForwardEdge(brick, currentSection, updateValue, inititalSide);
                break;
            }
            case UpdateEdgeContainer::SUB_TYPE:
            {
                processUpdateSubForwardEdge(brick, currentSection, updateValue, inititalSide);
                break;
            }
            case UpdateEdgeContainer::DELETE_TYPE:
            {
                processUpdateDeleteForwardEdge(brick, currentSection, forwardEdgeSectionId, inititalSide);
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
learningForwardEdgeSection(Brick* brick,
                           ForwardEdgeSection *currentSection,
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

                currentSection->forwardEdges[24].weight += currentSideWeight;
                uint32_t edgeSectionId = currentSection->forwardEdges[24].targetId;

                if(edgeSectionId == UNINIT_STATE_32)
                {
                    edgeSectionId =  addEmptyEdgeSection(brick, forwardEdgeSectionId);
                    assert(edgeSectionId != UNINIT_STATE_32);
                    currentSection->forwardEdges[24].targetId = edgeSectionId;
                }

                processEdgeSection(brick,
                                   edgeSectionId,
                                   currentSection->forwardEdges[24].weight);
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
                if(currentSection->forwardEdges[side].targetId == UNINIT_STATE_32
                        && currentSection->forwardEdges[side].weight == 0.0f)
                {
                    // send new learning-edge
                    LearingEdgeContainer newEdge;
                    newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
                    newEdge.weight = currentSideWeight;
                    sendData(brick, side, newEdge);
                }

                currentSection->forwardEdges[side].weight += currentSideWeight;
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
    DataConnection* connection = &brick->dataConnections[FORWARDEDGE_DATA];
    ForwardEdgeSection* currentSection = &getForwardEdgeBlock(connection)[forwardEdgeSectionId];

    if(currentSection->status != ACTIVE_SECTION) {
        return;
    }

    // process learning, if the incoming weight is too big
    const float totalWeight = currentSection->getTotalWeight();
    float ratio = weight / totalWeight;
    if(ratio > 1.0f)
    {
        learningForwardEdgeSection(brick,
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
        const ForwardEdge tempForwardEdge = currentSection->forwardEdges[sideCounter];
        if(tempForwardEdge.weight <= 0.0f) {
            continue;
        }

        if(sideCounter == 24)
        {
            assert(tempForwardEdge.targetId != UNINIT_STATE_32);
            processEdgeSection(brick, tempForwardEdge.targetId, tempForwardEdge.weight);
        }
        else
        {
            if(tempForwardEdge.targetId != UNINIT_STATE_32)
            {
                // normal external edge
                ForwardEdgeContainer newEdge;
                newEdge.targetEdgeSectionId = tempForwardEdge.targetId;
                newEdge.weight = tempForwardEdge.weight * ratio;
                sendData(brick, sideCounter, newEdge);
            }
            else
            {
                // send pendinge-edge if the learning-step is not finished
                PendingEdgeContainer newEdge;
                newEdge.weight = tempForwardEdge.weight * ratio;
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
processLerningForwardEdge(Brick* brick,
                          const uint32_t sourceEdgeSectionId,
                          const float weight,
                          const uint8_t initSide,
                          float* weightMap)
{
    const uint32_t targetEdgeSectionId = addEmptyForwardEdgeSection(brick,
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
processPendingForwardEdge(Brick* brick,
                          const uint32_t sourceId,
                          const uint8_t sourceSide,
                          const float weight,
                          float *weightMap)
{
    DataConnection* connection = &brick->dataConnections[FORWARDEDGE_DATA];

    const uint32_t numberOfForwardEdgeSections = connection->numberOfItems;
    ForwardEdgeSection* forwardEnd = getForwardEdgeBlock(connection);
    ForwardEdgeSection* forwardStart = &forwardEnd[numberOfForwardEdgeSections - 1];

    // beginn wigh the last forward-edge-section
    uint32_t forwardEdgeSectionId = numberOfForwardEdgeSections - 1;

    // search for the forward-edge-section with the same source-id
    // go backwards through the array, because the target-sections is nearly the end of the array
    for(ForwardEdgeSection* forwardEdgeSection = forwardStart;
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

}

#endif // NODEBRICKMETHODS_H
