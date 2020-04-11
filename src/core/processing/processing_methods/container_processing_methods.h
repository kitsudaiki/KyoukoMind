﻿/**
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
#include <root_object.h>

#include <core/objects/container_definitions.h>
#include <core/processing/processing_methods/brick_item_methods.h>
#include <core/processing/processing_methods/synapse_methods.h>

#include <core/objects/brick.h>

namespace KyoukoMind
{

//==================================================================================================

/**
 * @brief NodeBrick::createNewEdge
 * @param currentSection
 * @param edgeSectionId
 */
inline void
createNewSynapse(Brick &brick,
                 SynapseSection &currentSection)
{
    const uint32_t targetNodeId = brick.randValue[brick.randValuePos];
    brick.randValuePos = (brick.randValuePos + 1) % 1024;
    const uint32_t somaDistance = brick.randValue[brick.randValuePos];
    brick.randValuePos = (brick.randValuePos + 1) % 1024;

    addSynapse(currentSection,
               brick.globalValues.globalMemorizingOffset,
               targetNodeId,
               somaDistance);
}

//==================================================================================================

/**
 * @brief EdgeBrick::checkAndDelete
 * @param currentSection
 * @param forwardEdgeSectionId
 */
inline bool
checkAndDelete(Brick &brick,
               EdgeSection &currentSection,
               const uint32_t forwardEdgeSectionId)
{
    if(currentSection.totalWeight < 0.1f)
    {
        if(currentSection.sourceId != UNINIT_STATE_32)
        {
            UpdateEdgeContainer newEdge;
            newEdge.updateType = UpdateEdgeContainer::DELETE_TYPE;
            newEdge.targetId = currentSection.sourceId;
            addObjectToStackBuffer(*brick.neighbors[currentSection.sourceSide].outgoingBuffer,
                                   &newEdge);
        }

        deleteDynamicItem(brick, EDGE_DATA, forwardEdgeSectionId);

        return true;
    }

    return false;
}

//==================================================================================================

/**
 * learing-process of the specific synapse-section
 *
 * @param currentSection synapse-section with should learn the new value
 * @param weight weight-difference to learn
 */
inline void
learningSynapseSection(Brick &brick,
                       SynapseSection &currentSection,
                       float weight)
{
    if(weight < NEW_SYNAPSE_BORDER) {
        return;
    }

    for(uint8_t i = 0; i < 3; i++)
    {
        const uint32_t choosePosition = brick.randValue[brick.randValuePos]
                                        % (currentSection.numberOfSynapses + 1);
        brick.randValuePos = (brick.randValuePos + 1) % 1024;

        const float currentSideWeight = brick.randWeight[brick.randWeightPos] * weight;
        brick.randWeightPos = (brick.randWeightPos + 1) % 999;
        assert(currentSideWeight >= 0.0f);

        // create new synapse if necessary
        if(choosePosition == currentSection.numberOfSynapses) {
            createNewSynapse(brick, currentSection);
        }

        // synapses, which are fully memorized, are not allowed to be overwritten!!!
        if(currentSection.synapses[choosePosition].memorize >= 0.99f) {
            continue;
        }

        // get node of the synapse
        Synapse* synapse = &currentSection.synapses[choosePosition];
        Node* nodeBuffer = static_cast<Node*>(brick.dataConnections[NODE_DATA].buffer.data);
        Node* node = &nodeBuffer[synapse->targetNodeId];

        // check, if therer is already too much input on the node
        const uint8_t tooHeight = nodeBuffer[synapse->targetNodeId].tooHigh;

        // calculate new value
        float newVal = 0.0f;
        newVal = brick.globalValues.globalLearningOffset
                * currentSideWeight
                * ((tooHeight * -2) + 1);

        // make sure it is not too height
        if(node->currentState + newVal > 1.1f * node->border)
        {
            const float diff = (node->currentState + newVal) - (1.1f * node->border);
            newVal -= diff;
        }

        currentSection.synapses[choosePosition].weight += newVal;
    }
}

//==================================================================================================

/**
* process of a specific edge-section of a brick
*
* @param edgeSectionId id of the edge-section within the current brick
* @param weight incoming weight-value
*/
inline void
processSynapseSection(Brick &brick,
                      const uint32_t synapseSectionId,
                      const float inputWeight)
{
    DataConnection* connection = &brick.dataConnections[SYNAPSE_DATA];
    assert(connection->inUse != 0);
    SynapseSection* currentSection = &getSynapseSectionBlock(connection)[synapseSectionId];
    assert(currentSection->status == ACTIVE_SECTION);

    learningSynapseSection(brick,
                           *currentSection,
                           inputWeight - currentSection->totalWeight);

    // limit ration to 1.0f
    float ratio = inputWeight / currentSection->totalWeight;
    const uint8_t tooBig = ratio > 1.0f;
    ratio -= tooBig * (1.0f + ratio);

    Node* nodes = static_cast<Node*>(brick.dataConnections[NODE_DATA].buffer.data);
    Synapse* end = currentSection->synapses + currentSection->numberOfSynapses;

    for(Synapse* synapse = currentSection->synapses;
        synapse < end;
        synapse++)
    {
        const Synapse tempSynapse = *synapse;
        nodes[tempSynapse.targetNodeId].currentState +=
                tempSynapse.weight
                * ratio
                * ((float)tempSynapse.somaDistance / (float)MAX_SOMA_DISTANCE);
        synapse->inProcess = nodes[tempSynapse.targetNodeId].active;
    }
}

//==================================================================================================

/**
 * @brief EdgeBrick::processUpdateSetEdge
 * @param currentSection
 * @param updateValue
 * @param inititalSide
 */
inline void
processUpdateSetEdge(Brick &brick,
                     EdgeSection &currentSection,
                     const float updateValue,
                     const uint8_t inititalSide)
{
    const uint16_t ok = updateValue > 0.0f;
    currentSection.edges[inititalSide].weight = ok * updateValue;

    if(currentSection.sourceId != UNINIT_STATE_32)
    {
        UpdateEdgeContainer newEdge;
        newEdge.updateValue = ok * updateValue;
        newEdge.targetId = currentSection.sourceId;
        addObjectToStackBuffer(*brick.neighbors[currentSection.sourceSide].outgoingBuffer,
                               &newEdge);
    }
}

//==================================================================================================

/**
 * @brief EdgeBrick::processUpdateSubEdge
 * @param currentSection
 * @param updateValue
 * @param inititalSide
 */
inline void
processUpdateSubEdge(Brick &brick,
                     EdgeSection &currentSection,
                     float updateValue,
                     const uint8_t inititalSide)
{
    updateEdgeWeight(currentSection, inititalSide, updateValue);

    if(currentSection.sourceId != UNINIT_STATE_32)
    {
        UpdateEdgeContainer replyEdge;
        replyEdge.updateType = UpdateEdgeContainer::SUB_TYPE;
        replyEdge.targetId = currentSection.sourceId;
        replyEdge.updateValue = updateValue;
        addObjectToStackBuffer(*brick.neighbors[currentSection.sourceSide].outgoingBuffer,
                               &replyEdge);
    }
}

//==================================================================================================

/**
 * @brief EdgeBrick::processUpdateDeleteEdge
 * @param currentSection
 * @param inititalSide
 */
inline void
processUpdateDeleteEdge(Brick &brick,
                        EdgeSection &currentSection,
                        const uint32_t forwardEdgeSectionId,
                        const uint8_t inititalSide)
{
    currentSection.totalWeight -= currentSection.edges[inititalSide].weight;
    currentSection.edges[inititalSide].weight = 0.0f;
    currentSection.edges[inititalSide].targetId = UNINIT_STATE_32;

    if(checkAndDelete(brick, currentSection, forwardEdgeSectionId) == false
            && currentSection.sourceId != UNINIT_STATE_32)
    {
        UpdateEdgeContainer newEdge;
        newEdge.updateType = UpdateEdgeContainer::SUB_TYPE;
        newEdge.updateValue = currentSection.edges[inititalSide].weight;
        newEdge.targetId = currentSection.sourceId;
        addObjectToStackBuffer(*brick.neighbors[currentSection.sourceSide].outgoingBuffer,
                               &newEdge);
    }
}

//==================================================================================================

/**
 * process status
 *
 * @param forwardEdgeSectionId forward-edge-section in the current brick
 * @param updateValue incoming diff-value
 * @param updateType delete connection to the new edge
 * @param inititalSide side where the status-value comes in
 */
inline void
processUpdateEdge(Brick &brick,
                  const UpdateEdgeContainer &edge,
                  const uint8_t inititalSide)
{
    DataConnection* connection = &brick.dataConnections[EDGE_DATA];
    assert(connection->inUse != 0);
    EdgeSection* currentSection = &getEdgeBlock(connection)[edge.targetId];
    assert(currentSection->status == ACTIVE_SECTION);

    switch(edge.updateType)
    {
        case UpdateEdgeContainer::SET_TYPE:
        {
            processUpdateSetEdge(brick, *currentSection, edge.updateValue, inititalSide);
            break;
        }
        case UpdateEdgeContainer::SUB_TYPE:
        {
            processUpdateSubEdge(brick, *currentSection, edge.updateValue, inititalSide);
            break;
        }
        case UpdateEdgeContainer::DELETE_TYPE:
        {
            processUpdateDeleteEdge(brick, *currentSection, edge.targetId, inititalSide);
            break;
        }
        default:
            break;
    }
}

//==================================================================================================

/**
 * create new edges for the current section
 *
 * @param currentSection pointer to the current section
 * @param forwardEdgeSectionId the id of the current section
 * @param weight weight with have to be consumed from the updated edges
 */
inline void
learningEdgeSection(Brick &brick,
                    EdgeSection* currentSection,
                    const uint32_t forwardEdgeSectionId,
                    const float weight)
{
    assert(currentSection->status == ACTIVE_SECTION);

    if(weight < 0.5f) {
        return;
    }

    for(uint8_t i = 0; i < 3; i++)
    {
        // TODO: offset +9 only for now
        const uint32_t side = (brick.randValue[brick.randValuePos] % 14) + 9;
        brick.randValuePos = (brick.randValuePos + 1) % 1024;

        // only process available edges
        if(currentSection->edges[side].available == 0) {
            continue;
        }

        const float currentSideWeight = brick.randWeight[brick.randWeightPos] * weight;
        brick.randWeightPos = (brick.randWeightPos + 1) % 999;
        assert(currentSideWeight >= 0.0f);

        currentSection->totalWeight += weight;

        if(side == 22)
        {
            currentSection->edges[side].weight += currentSideWeight;
            uint32_t edgeSectionId = currentSection->edges[side].targetId;
            if(edgeSectionId == UNINIT_STATE_32)
            {
                edgeSectionId = addEmptySynapseSection(brick, forwardEdgeSectionId);
                assert(edgeSectionId != UNINIT_STATE_32);
                currentSection->edges[side].targetId = edgeSectionId;
            }
        }
        else
        {
            // brick-external lerning
            const uint32_t targetId = currentSection->edges[side].targetId;
            if(targetId == UNINIT_STATE_32)
            {
                // send new learning-edge
                LearingEdgeContainer newEdge;
                newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
                newEdge.weight = currentSideWeight;
                addObjectToStackBuffer(*brick.neighbors[side].outgoingBuffer,
                                       &newEdge);
            }
        }
    }
}

//==================================================================================================

/**
 * processEdgeForwardSection
 *
 * @param forwardEdgeSectionId target edge-section of the incoming edge
 * @param weight incoming edge-weight
 */
inline void
processEdgeForwardSection(Brick &brick,
                          const EdgeContainer &edge)
{
    DataConnection* connection = &brick.dataConnections[EDGE_DATA];
    assert(connection->inUse != 0);
    EdgeSection* edgeSection = &getEdgeBlock(connection)[edge.targetEdgeSectionId];
    assert(edgeSection->status == ACTIVE_SECTION);

    // process learning, if the incoming weight is too big
    const float totalWeight = edgeSection->totalWeight;
    learningEdgeSection(brick,
                        edgeSection,
                        edge.targetEdgeSectionId,
                        edge.weight - totalWeight);

    // limit ration to 1.0f
    float ratio = edge.weight / totalWeight;
    const uint8_t tooBig = ratio > 1.0f;
    ratio -= tooBig * (1.0f + ratio);

    // iterate over all forward-edges in the current section
    for(uint8_t side = 0; side < 23; side++)
    {
        const Edge tempEdge = edgeSection->edges[side];
        if(tempEdge.available == 0
                || tempEdge.weight <= 0.0f) {
            continue;
        }

        if(side == 22)
        {
            assert(tempEdge.targetId != UNINIT_STATE_32);
            processSynapseSection(brick, tempEdge.targetId, tempEdge.weight * ratio);
        }
        else
        {
            if(tempEdge.targetId != UNINIT_STATE_32)
            {
                // normal external edge
                EdgeContainer newEdge;
                newEdge.targetEdgeSectionId = tempEdge.targetId;
                newEdge.weight = tempEdge.weight * ratio;
                addObjectToStackBuffer(*brick.neighbors[side].outgoingBuffer,
                                       &newEdge);
            }
            else
            {
                // send pendinge-edge if the learning-step is not finished
                PendingEdgeContainer newEdge;
                newEdge.weight = tempEdge.weight * ratio;
                newEdge.sourceEdgeSectionId = edge.targetEdgeSectionId;
                newEdge.sourceSide = 23 - side;
                addObjectToStackBuffer(*brick.neighbors[side].outgoingBuffer,
                                       &newEdge);
            }
        }
    }
}

//==================================================================================================

/**
 * process axon
 *
 * @param targetId section-target-id at the end of the axon
 * @param path rest of the path of the axon
 * @param weight weight of the axon
 */
inline void
processAxon(Brick &brick,
            const AxonEdgeContainer &edge)
{
    if(edge.targetBrickPath != 0)
    {
        // forward axon the the next in the path
        AxonEdgeContainer newEdge;
        newEdge.targetBrickPath = edge.targetBrickPath >> 5;
        newEdge.weight = edge.weight * brick.globalValues.globalGlia;
        newEdge.targetAxonId = edge.targetAxonId;
        const uint8_t side = edge.targetBrickPath & 0x1F;
        addObjectToStackBuffer(*brick.neighbors[side].outgoingBuffer,
                               &newEdge);
    }
    else
    {
        // if target brick reached, update the state of the target-axon with the edge
        EdgeContainer newEdge;
        newEdge.targetEdgeSectionId = edge.targetAxonId;
        newEdge.weight = edge.weight;
        processEdgeForwardSection(brick, newEdge);
    }
}

//==================================================================================================

/**
 * process learning-edge to create a new forward-edge
 *
 * @param sourceEdgeSectionId id of the source-edge-section within the last brick
 * @param weight weight of the new edge
 * @param initSide side of the incoming message
 */
inline void
processLerningEdge(Brick &brick,
                   const LearingEdgeContainer &edge,
                   const uint8_t initSide)
{
    const uint32_t targetEdgeId = addEmptyEdgeSection(brick, initSide, edge.sourceEdgeSectionId);

    // create reply-message
    LearningEdgeReplyContainer reply;
    reply.sourceEdgeSectionId = edge.sourceEdgeSectionId;
    reply.targetEdgeSectionId = targetEdgeId;
    addObjectToStackBuffer(*brick.neighbors[initSide].outgoingBuffer,
                           &reply);

    EdgeContainer newEdge;
    newEdge.targetEdgeSectionId = targetEdgeId;
    newEdge.weight = edge.weight;
    processEdgeForwardSection(brick, newEdge);
}

//==================================================================================================

/**
 * process pending-edge in the cycle after the learning-edge
 *
 * @param sourceId source-section-id of the pending-edge
 * @param sourceSide side of the incoming pending-edge
 * @param weight weight of the pending-edge
 */
inline void
processPendingEdge(Brick &brick,
                   const PendingEdgeContainer &edge)
{
    DataConnection* connection = &brick.dataConnections[EDGE_DATA];

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
                && edge.sourceEdgeSectionId == forwardEdgeSection->sourceId
                && edge.sourceSide == forwardEdgeSection->sourceSide)
        {
            EdgeContainer newEdge;
            newEdge.targetEdgeSectionId = forwardEdgeSectionId;
            newEdge.weight = edge.weight;
            processEdgeForwardSection(brick, newEdge);
        }
        forwardEdgeSectionId--;
    }
}

//==================================================================================================

/**
 * @brief processDirectEdge
 * @param brick
 * @param edge
 */
inline void
processDirectEdge(Brick &brick,
                  const DirectEdgeContainer &edge)
{
    Node* nodes = static_cast<Node*>(brick.dataConnections[NODE_DATA].buffer.data);
    Node* node = &nodes[edge.targetNodeId];
    node->currentState = edge.weight;
}

//==================================================================================================

/**
 * @brief processDirectEdge
 * @param brick
 * @param edge
 */
inline void
processLearningEdgeReply(Brick &brick,
                         const LearningEdgeReplyContainer &edge,
                         const uint8_t side)
{
    EdgeSection* edgeSections = getEdgeBlock(&brick.dataConnections[EDGE_DATA]);
    edgeSections[edge.sourceEdgeSectionId].edges[side].targetId =
            edge.targetEdgeSectionId;
}

//==================================================================================================

} // namespace KyoukoMind

#endif // MESSAGE_PROCESSING_METHODS_H
