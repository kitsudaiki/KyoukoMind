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
               EdgeSection &edgeSection,
               const uint32_t edgeSectionId)
{
    if(edgeSection.totalWeight < 0.1f)
    {
        if(edgeSection.sourceId != UNINIT_STATE_32)
        {
            UpdateEdgeContainer newContainer;
            newContainer.updateType = UpdateEdgeContainer::DELETE_TYPE;
            newContainer.targetId = edgeSection.sourceId;
            addObjectToStackBuffer(*brick.neighbors[edgeSection.sourceSide].outgoingBuffer,
                                   &newContainer);
        }

        deleteDynamicItem(brick, EDGE_DATA, edgeSectionId);

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

        if(brick.globalValues.globalLearningOffset < 0.01f) {
            return;
        }

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
        currentSection.totalWeight += abs(newVal);
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
    std::vector<SynapseSection*> debugList;
    for(uint32_t i = 0; i < connection->numberOfItems; i++)
    {
        debugList.push_back(&getSynapseSectionBlock(connection)[i]);
    }
    assert(currentSection->status == ACTIVE_SECTION);

    learningSynapseSection(brick,
                           *currentSection,
                           inputWeight - currentSection->totalWeight);

    // limit ration to 1.0f
    float ratio = inputWeight / currentSection->totalWeight;
    if(ratio > 1.0f) {
        ratio = 1.0f;
    }

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
                     EdgeSection &edgeSection,
                     const float newValue,
                     const uint8_t inititalSide)
{
    assert(newValue >= 0.0f);

    const float diff = edgeSection.edges[inititalSide].weight - newValue;
    edgeSection.edges[inititalSide].weight = newValue;

    if(edgeSection.sourceId != UNINIT_STATE_32)
    {
        assert(edgeSection.sourceSide != 0);

        UpdateEdgeContainer newContainer;
        newContainer.type = UpdateEdgeContainer::SUB_TYPE;
        newContainer.updateValue = diff;
        newContainer.targetId = edgeSection.sourceId;
        addObjectToStackBuffer(*brick.neighbors[edgeSection.sourceSide].outgoingBuffer,
                               &newContainer);
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
                     EdgeSection &edgeSection,
                     const float updateValue,
                     const uint8_t inititalSide)
{
    edgeSection.totalWeight -= edgeSection.edges[inititalSide].weight;
    edgeSection.edges[inititalSide].weight = 0.0000001f;

    if(edgeSection.sourceId != UNINIT_STATE_32)
    {
        assert(edgeSection.sourceSide != 0);

        UpdateEdgeContainer newContainer;
        newContainer.updateType = UpdateEdgeContainer::SUB_TYPE;
        newContainer.targetId = edgeSection.sourceId;
        newContainer.updateValue = updateValue;
        addObjectToStackBuffer(*brick.neighbors[edgeSection.sourceSide].outgoingBuffer,
                               &newContainer);
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
                        EdgeSection &edgeSection,
                        const uint32_t edgeSectionId,
                        const uint8_t inititalSide)
{
    const float temp = edgeSection.edges[inititalSide].weight;
    edgeSection.totalWeight -= temp;
    edgeSection.edges[inititalSide].weight = 0.0000001f;
    edgeSection.edges[inititalSide].targetId = UNINIT_STATE_32;

    if(checkAndDelete(brick, edgeSection, edgeSectionId) == false
            && edgeSection.sourceId != UNINIT_STATE_32)
    {
        assert(edgeSection.sourceSide != 0);

        UpdateEdgeContainer newContainer;
        newContainer.updateType = UpdateEdgeContainer::SUB_TYPE;
        newContainer.updateValue = temp;
        newContainer.targetId = edgeSection.sourceId;
        addObjectToStackBuffer(*brick.neighbors[edgeSection.sourceSide].outgoingBuffer,
                               &newContainer);
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
                  const UpdateEdgeContainer &container,
                  const uint8_t inititalSide)
{
    DataConnection* connection = &brick.dataConnections[EDGE_DATA];
    assert(connection->inUse != 0);
    EdgeSection* edgeSection = &getEdgeBlock(connection)[container.targetId];

    // here no assert, because based on the async processing it is possible to get a
    // update-message, after the section was deleted
    if(edgeSection->status != ACTIVE_SECTION) {
        return;
    }

    switch(container.updateType)
    {
        case UpdateEdgeContainer::SUB_TYPE:
        {
            processUpdateSubEdge(brick, *edgeSection, container.updateValue, inititalSide);
            break;
        }
        case UpdateEdgeContainer::DELETE_TYPE:
        {
            processUpdateDeleteEdge(brick, *edgeSection, container.targetId, inititalSide);
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
                    EdgeSection* edgeSection,
                    const uint32_t edgeSectionId,
                    const float weight)
{
    assert(edgeSection->status == ACTIVE_SECTION);

    if(weight < 0.5f) {
        return;
    }

    for(uint8_t i = 0; i < 3; i++)
    {
        // TODO: offset +9 only for now
        const uint32_t side = (brick.randValue[brick.randValuePos] % 14) + 9;
        brick.randValuePos = (brick.randValuePos + 1) % 1024;

        // only process available edges
        if(edgeSection->edges[side].available == 0) {
            continue;
        }

        // get rand weight-value
        const float currentSideWeight = brick.randWeight[brick.randWeightPos] * weight;
        brick.randWeightPos = (brick.randWeightPos + 1) % 999;
        assert(currentSideWeight >= 0.0f);

        // update values of section
        edgeSection->totalWeight += weight;
        edgeSection->edges[side].weight += currentSideWeight;

        uint32_t targetId = edgeSection->edges[side].targetId;
        if(targetId == UNINIT_STATE_32)
        {
            if(side == 22)
            {
                targetId = addEmptySynapseSection(brick, edgeSectionId);
                assert(targetId != UNINIT_STATE_32);
                DataConnection* connection = &brick.dataConnections[SYNAPSE_DATA];
                assert(connection->inUse != 0);
                SynapseSection* synapseSection = &getSynapseSectionBlock(connection)[targetId];
                assert(synapseSection->status == ACTIVE_SECTION);
                edgeSection->edges[22].targetId = targetId;
            }
            else
            {
                // send new learning-edge
                LearingEdgeContainer newContainer;
                newContainer.sourceEdgeSectionId = edgeSectionId;
                newContainer.weight = currentSideWeight;
                addObjectToStackBuffer(*brick.neighbors[side].outgoingBuffer,
                                       &newContainer);
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
                          const EdgeContainer &container)
{
    DataConnection* connection = &brick.dataConnections[EDGE_DATA];
    assert(connection->inUse != 0);
    EdgeSection* edgeSection = &getEdgeBlock(connection)[container.targetEdgeSectionId];
    assert(edgeSection->status == ACTIVE_SECTION);

    // process learning, if the incoming weight is too big
    const float totalWeight = edgeSection->totalWeight;
    learningEdgeSection(brick,
                        edgeSection,
                        container.targetEdgeSectionId,
                        container.weight - totalWeight);

    // limit ration to 1.0f
    float ratio = container.weight / totalWeight;
    if(ratio > 1.0f) {
        ratio = 1.0f;
    }

    // iterate over all forward-edges in the current section
    for(uint8_t side = 2; side < 23; side++)
    {
        const Edge tempEdge = edgeSection->edges[side];
        if(tempEdge.available == 0
                || tempEdge.weight <= 0.0001f) {
            continue;
        }

        assert(tempEdge.weight >= 0.0f);

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
                EdgeContainer newContainer;
                newContainer.targetEdgeSectionId = tempEdge.targetId;
                newContainer.weight = tempEdge.weight * ratio;
                assert(newContainer.weight >= 0.0f);
                addObjectToStackBuffer(*brick.neighbors[side].outgoingBuffer,
                                       &newContainer);
            }
            else
            {
                // send pendinge-edge if the learning-step is not finished
                PendingEdgeContainer newContainer;
                newContainer.weight = tempEdge.weight * ratio;
                assert(newContainer.weight >= 0);
                newContainer.sourceEdgeSectionId = container.targetEdgeSectionId;
                newContainer.sourceSide = 23 - side;
                addObjectToStackBuffer(*brick.neighbors[side].outgoingBuffer,
                                       &newContainer);
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
            const AxonEdgeContainer &container)
{
    if(container.targetBrickPath != 0)
    {
        // forward axon the the next in the path
        AxonEdgeContainer newContainer;
        newContainer.targetBrickPath = container.targetBrickPath >> 5;
        newContainer.weight = container.weight * brick.globalValues.globalGlia;
        newContainer.targetAxonId = container.targetAxonId;
        const uint8_t side = container.targetBrickPath & 0x1F;
        addObjectToStackBuffer(*brick.neighbors[side].outgoingBuffer,
                               &newContainer);
    }
    else
    {
        // if target brick reached, update the state of the target-axon with the edge
        EdgeContainer newContainier;
        newContainier.targetEdgeSectionId = container.targetAxonId;
        newContainier.weight = container.weight;
        processEdgeForwardSection(brick, newContainier);
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
                   const LearingEdgeContainer &container,
                   const uint8_t initSide)
{
    assert(initSide != 0);
    const uint32_t targetEdgeId = addEmptyEdgeSection(brick,
                                                      initSide,
                                                      container.sourceEdgeSectionId);
    assert(targetEdgeId != UNINIT_STATE_32);

    DataConnection* connection = &brick.dataConnections[EDGE_DATA];
    assert(connection->inUse != 0);
    EdgeSection* edgeSection = &getEdgeBlock(connection)[targetEdgeId];
    assert(edgeSection->status == ACTIVE_SECTION);
    assert(edgeSection->sourceSide != 0);

    // create reply-message
    LearningEdgeReplyContainer reply;
    reply.sourceEdgeSectionId = container.sourceEdgeSectionId;
    reply.targetEdgeSectionId = targetEdgeId;
    addObjectToStackBuffer(*brick.neighbors[initSide].outgoingBuffer,
                           &reply);

    EdgeContainer newContainer;
    newContainer.targetEdgeSectionId = targetEdgeId;
    newContainer.weight = container.weight;
    assert(newContainer.weight >= 0.0f);
    processEdgeForwardSection(brick, newContainer);
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
                   const PendingEdgeContainer &container)
{
    assert(container.sourceSide != 0);
    DataConnection* connection = &brick.dataConnections[EDGE_DATA];
    assert(connection->inUse != 0);

    const uint32_t numberOfEdgeSections = connection->numberOfItems;
    EdgeSection* forwardEnd = getEdgeBlock(connection);
    EdgeSection* forwardStart = &forwardEnd[numberOfEdgeSections - 1];

    // beginn wigh the last forward-edge-section
    uint32_t forwardEdgeSectionId = numberOfEdgeSections - 1;

    // search for the forward-edge-section with the same source-id
    // go backwards through the array, because the target-sections is nearly the end of the array
    for(EdgeSection* edgeSection = forwardStart;
        edgeSection >= forwardEnd;
        edgeSection--)
    {
        if(edgeSection->status == ACTIVE_SECTION)
        {
            if(edgeSection->sourceId != UNINIT_STATE_32 && edgeSection->sourceId != 0) {
                assert(edgeSection->sourceSide != 0);
            }
            assert(container.sourceSide != 0);
        }

        // if found, then process the pending-edge als normal forward-edge
        if(edgeSection->status == ACTIVE_SECTION
                && container.sourceEdgeSectionId == edgeSection->sourceId
                && container.sourceSide == edgeSection->sourceSide)
        {
            EdgeContainer newContainer;
            newContainer.targetEdgeSectionId = forwardEdgeSectionId;
            newContainer.weight = container.weight;
            assert(newContainer.weight >= 0.0f);
            processEdgeForwardSection(brick, newContainer);
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
                  const DirectEdgeContainer &container)
{
    Node* nodes = static_cast<Node*>(brick.dataConnections[NODE_DATA].buffer.data);
    Node* node = &nodes[container.targetNodeId];
    node->currentState = container.weight;
}

//==================================================================================================

/**
 * @brief processDirectEdge
 * @param brick
 * @param edge
 */
inline void
processLearningEdgeReply(Brick &brick,
                         const LearningEdgeReplyContainer &container,
                         const uint8_t side)
{
    EdgeSection* edgeSections = getEdgeBlock(&brick.dataConnections[EDGE_DATA]);
    edgeSections[container.sourceEdgeSectionId].edges[side].targetId =
            container.targetEdgeSectionId;
}

//==================================================================================================

} // namespace KyoukoMind

#endif // MESSAGE_PROCESSING_METHODS_H
