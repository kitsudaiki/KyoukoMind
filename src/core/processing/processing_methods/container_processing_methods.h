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
    Synapse newSynapse;

    newSynapse.targetNodeId = static_cast<uint16_t>(rand()) % NUMBER_OF_NODES_PER_BRICK;
    newSynapse.memorize = brick.globalValues.globalMemorizingOffset;
    newSynapse.somaDistance = static_cast<uint8_t>(rand() % (MAX_SOMA_DISTANCE - 1)) + 1;

    addSynapse(currentSection, newSynapse);
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
    if(currentSection.sourceId != UNINIT_STATE_32
            && currentSection.activeEdges == 0)
    {
        UpdateEdgeContainer newEdge;
        newEdge.updateType = UpdateEdgeContainer::DELETE_TYPE;
        newEdge.targetId = currentSection.sourceId;
        addObjectToStackBuffer(*brick.neighbors[currentSection.sourceSide].outgoingBuffer,
                               &newEdge);

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
    while(weight > 0.0f)
    {
        // calculate new value
        float value = weight;
        if(value > MINIMUM_NEW_EDGE_BODER * 2) {
            value /= 2.0f;
        }
        weight -= value;

        // choose synapse
        uint32_t choosePosition = static_cast<uint32_t>(rand())
                                  % (currentSection.numberOfSynapses + 1);

        // create new synapse if necessary
        if(choosePosition == currentSection.numberOfSynapses)
        {
            createNewSynapse(brick, currentSection);
        }
        // synapses, which are fully memorized, are not allowed to be overwritten!
        else if(currentSection.synapses[choosePosition].memorize >= 0.99f)
        {
            choosePosition = currentSection.numberOfSynapses;
            createNewSynapse(brick, currentSection);
        }

        Synapse* synapse = &currentSection.synapses[choosePosition];
        Node* nodeBuffer = static_cast<Node*>(brick.dataConnections[NODE_DATA].buffer.data);
        Node* node = &nodeBuffer[synapse->targetNodeId];

        const uint8_t tooHeight = nodeBuffer[synapse->targetNodeId].tooHigh;

        float newVal = 0.0f;
        if(brick.isOutputBrick == 0)
        {
            newVal = brick.globalValues.globalLearningOffset * value * ((tooHeight * -2) + 1);
            // make sure it is not too height
            if(node->currentState + newVal > 1.1f * node->border)
            {
                const float diff = (node->currentState + newVal) - (1.1f * node->border);
                newVal -= diff;
            }
        }
        else
        {
            newVal = brick.learningOverride * value;
        }

        currentSection.synapses[choosePosition].weight += newVal;
    }
}

//==================================================================================================

/**
 * @brief NodeBrick::checkEdge
 * @param currentSection
 * @param weight
 * @return
 */
inline float
checkSynapse(Brick &brick,
             SynapseSection &currentSection,
             const float weight)
{
    const float totalWeight = currentSection.totalWeight;
    const float ratio = weight / totalWeight;

    if(ratio > 1.0f)
    {
        if(weight - totalWeight >= NEW_SYNAPSE_BORDER
                && brick.globalValues.globalLearningOffset > 0.01f)
        {
            learningSynapseSection(brick, currentSection, weight - totalWeight);
        }
        return 1.0f;
    }
    return ratio;
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

    // precheck
    if(inputWeight < 0.5f) {
        return;
    }

    const float ratio = checkSynapse(brick, *currentSection, inputWeight);

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

    UpdateEdgeContainer newEdge;
    newEdge.updateValue = ok * updateValue;
    currentSection.edges[inititalSide].weight = ok * updateValue;

    if(currentSection.sourceId != UNINIT_STATE_32)
    {
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
    UpdateEdgeContainer newEdge;
    newEdge.updateValue = updateValue;

    currentSection.edges[inititalSide].weight -= updateValue;

    UpdateEdgeContainer replyEdge;
    replyEdge.updateType = UpdateEdgeContainer::SET_TYPE;
    replyEdge.targetId = currentSection.edges[inititalSide].targetId;
    replyEdge.updateValue = currentSection.edges[inititalSide].weight;

    newEdge.targetId = currentSection.sourceId;
    addObjectToStackBuffer(*brick.neighbors[currentSection.sourceSide].outgoingBuffer,
                           &replyEdge);

    if(currentSection.sourceId != UNINIT_STATE_32)
    {
        newEdge.targetId = currentSection.sourceId;
        addObjectToStackBuffer(*brick.neighbors[currentSection.sourceSide].outgoingBuffer,
                               &newEdge);
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
    UpdateEdgeContainer newEdge;
    newEdge.updateValue = currentSection.edges[inititalSide].weight;

    if(inititalSide < 23)
    {
        currentSection.edges[inititalSide].weight = 0.0f;
        currentSection.edges[inititalSide].targetId = UNINIT_STATE_32;
    }

    if(checkAndDelete(brick, currentSection, forwardEdgeSectionId) == false)
    {
        if(currentSection.sourceId != UNINIT_STATE_32)
        {
            newEdge.targetId = currentSection.sourceId;
            addObjectToStackBuffer(*brick.neighbors[currentSection.sourceSide].outgoingBuffer,
                                   &newEdge);
        }
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

    if(weight >= 0.5f)
    {
        for(uint8_t i = 0; i < 3; i++)
        {
            const uint32_t position = brick.randValue[brick.randValuePos]
                                      % currentSection->activeEdges;
            brick.randValuePos = (brick.randValuePos + 1) % 1024;

            const uint8_t side = currentSection->edges[position].side;

            const float currentSideWeight = brick.randWeight[brick.randWeightPos] * weight;
            brick.randWeightPos = (brick.randWeightPos + 1) % 999;
            assert(currentSideWeight >= 0.0f);

            currentSection->totalWeight += weight;

            if(side == 22)
            {
                currentSection->edges[position].weight += currentSideWeight;
                uint32_t edgeSectionId = currentSection->edges[position].targetId;
                if(edgeSectionId == UNINIT_STATE_32)
                {
                    edgeSectionId = addEmptySynapseSection(brick, forwardEdgeSectionId);
                    assert(edgeSectionId != UNINIT_STATE_32);
                    currentSection->edges[position].targetId = edgeSectionId;
                }

                processSynapseSection(brick,
                                      edgeSectionId,
                                      currentSection->edges[position].weight);
            }
            else
            {
                // brick-external lerning
                const uint32_t targetId = currentSection->edges[position].targetId;
                if(targetId == UNINIT_STATE_32)
                {
                    // send new learning-edge
                    LearingEdgeContainer newEdge;
                    newEdge.sourceEdgeSectionId = forwardEdgeSectionId;
                    newEdge.weight = currentSideWeight;
                    addObjectToStackBuffer(*brick.neighbors[position].outgoingBuffer,
                                           &newEdge);
                }
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
    EdgeSection* currentSection = &getEdgeBlock(connection)[edge.targetEdgeSectionId];
    assert(currentSection->status == ACTIVE_SECTION);

    // process learning, if the incoming weight is too big
    const float totalWeight = currentSection->totalWeight;
    float ratio = edge.weight / totalWeight;
    if(ratio > 1.0f)
    {
        learningEdgeSection(brick,
                            currentSection,
                            edge.targetEdgeSectionId,
                            edge.weight - totalWeight);
        ratio = 1.0f;

        if(checkAndDelete(brick, *currentSection, edge.targetEdgeSectionId)) {
            return;
        }
    }

    // iterate over all forward-edges in the current section
    for(uint8_t position = 0; position < currentSection->activeEdges; position++)
    {
        const Edge tempEdge = currentSection->edges[position];
        if(tempEdge.weight <= 0.0f) {
            continue;
        }

        if(tempEdge.side == 22)
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
                addObjectToStackBuffer(*brick.neighbors[tempEdge.side].outgoingBuffer,
                                       &newEdge);
            }
            else
            {
                // send pendinge-edge if the learning-step is not finished
                PendingEdgeContainer newEdge;
                newEdge.weight = tempEdge.weight * ratio;
                newEdge.sourceEdgeSectionId = edge.targetEdgeSectionId;
                newEdge.sourceSide = 23 - tempEdge.side;
                addObjectToStackBuffer(*brick.neighbors[tempEdge.side].outgoingBuffer,
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
        newEdge.targetBrickPath = edge.targetBrickPath / 32;
        newEdge.weight = edge.weight * brick.globalValues.globalGlia;
        newEdge.targetAxonId = edge.targetAxonId;
        const uint8_t side = edge.targetBrickPath % 32;
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
