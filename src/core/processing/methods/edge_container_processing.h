/**
 *  @file    message_processing_methods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#ifndef EDGE_CONTAINER_PROCESSING_H
#define EDGE_CONTAINER_PROCESSING_H

#include <common.h>
#include <kyouko_root.h>

#include <core/objects/container_definitions.h>
#include <core/methods/brick_item_methods.h>
#include <core/methods/synapse_methods.h>
#include <core/methods/data_connection_methods.h>
#include <core/methods/network_segment_methods.h>

#include <core/objects/brick.h>
#include <core/objects/network_segment.h>

namespace KyoukoMind
{

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
            Kitsunemimi::addObject_StackBuffer(
                        *brick.neighbors[edgeSection.sourceSide].outgoingBuffer,
                        &newContainer);
        }

        deleteDynamicItem(brick.edges, edgeSectionId);

        return true;
    }

    return false;
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
        newContainer.updateType = UpdateEdgeContainer::SUB_TYPE;
        newContainer.updateValue = diff;
        newContainer.targetId = edgeSection.sourceId;
        Kitsunemimi::addObject_StackBuffer(
                    *brick.neighbors[edgeSection.sourceSide].outgoingBuffer,
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
        Kitsunemimi::addObject_StackBuffer(
                    *brick.neighbors[edgeSection.sourceSide].outgoingBuffer,
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
        Kitsunemimi::addObject_StackBuffer(
                    *brick.neighbors[edgeSection.sourceSide].outgoingBuffer,
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
    assert(brick.edges.inUse != 0);
    EdgeSection* edgeSection = &getEdgeBlock(brick)[container.targetId];

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
learningEdgeSection(NetworkSegment &segment,
                    Brick &brick,
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
                const uint64_t id = addEmptySynapseSection(segment, edgeSectionId, brick.brickId);
                targetId = static_cast<uint32_t>(id);
                assert(targetId != UNINIT_STATE_32);
                SynapseSection* synapseSection = &getSynapseSectionBlock(segment)[targetId];
                assert(synapseSection->status == ACTIVE_SECTION);
                // TODO: push to GPU
                edgeSection->edges[22].targetId = targetId;
            }
            else
            {
                // send new learning-edge
                LearingEdgeContainer newContainer;
                newContainer.sourceEdgeSectionId = edgeSectionId;
                newContainer.weight = currentSideWeight;
                Kitsunemimi::addObject_StackBuffer(*brick.neighbors[side].outgoingBuffer,
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
processEdgeForwardSection(NetworkSegment &segment,
                          Brick &brick,
                          const EdgeContainer &container)
{
    assert(brick.edges.inUse != 0);
    EdgeSection* edgeSection = &getEdgeBlock(brick)[container.targetEdgeSectionId];
    if(edgeSection->status != ACTIVE_SECTION) {
        return;
    }

    // process learning, if the incoming weight is too big
    const float totalWeight = edgeSection->totalWeight;
    learningEdgeSection(segment,
                        brick,
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
        if(tempEdge.available == 0) {
            continue;
        }

        if(side == 22)
        {
            if(tempEdge.targetId == UNINIT_STATE_32){
                continue;
            }

            void* transferData = segment.synapseEdges.buffer.data;
            SynapseTransfer* synapseTransfer = static_cast<SynapseTransfer*>(transferData);
            synapseTransfer[tempEdge.targetId].weight = tempEdge.weight * ratio;
            synapseTransfer[tempEdge.targetId].sourceEdgeId = container.targetEdgeSectionId;
            synapseTransfer[tempEdge.targetId].brickId = brick.brickId;
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
                Kitsunemimi::addObject_StackBuffer(*brick.neighbors[side].outgoingBuffer,
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
                Kitsunemimi::addObject_StackBuffer(*brick.neighbors[side].outgoingBuffer,
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
processAxon(NetworkSegment &segment,
            Brick &brick,
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
        Kitsunemimi::addObject_StackBuffer(*brick.neighbors[side].outgoingBuffer,
                                           &newContainer);
    }
    else
    {
        // if target brick reached, update the state of the target-axon with the edge
        EdgeContainer newContainier;
        newContainier.targetEdgeSectionId = container.targetAxonId;
        newContainier.weight = container.weight;
        processEdgeForwardSection(segment,
                                  brick,
                                  newContainier);
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
processLerningEdge(NetworkSegment &segment,
                   Brick &brick,
                   const LearingEdgeContainer &container,
                   const uint8_t initSide)
{
    assert(initSide != 0);
    const uint64_t targetEdgeId = addEmptyEdgeSection(brick,
                                                      initSide,
                                                      container.sourceEdgeSectionId);
    assert(targetEdgeId != UNINIT_STATE_32);

    assert(brick.edges.inUse != 0);
    EdgeSection* edgeSection = &getEdgeBlock(brick)[targetEdgeId];
    assert(edgeSection->status == ACTIVE_SECTION);
    assert(edgeSection->sourceSide != 0);

    // create reply-message
    LearningEdgeReplyContainer reply;
    reply.sourceEdgeSectionId = container.sourceEdgeSectionId;
    reply.targetEdgeSectionId = static_cast<uint32_t>(targetEdgeId);
    Kitsunemimi::addObject_StackBuffer(*brick.neighbors[initSide].outgoingBuffer,
                                       &reply);

    EdgeContainer newContainer;
    newContainer.targetEdgeSectionId = static_cast<uint32_t>(targetEdgeId);
    newContainer.weight = container.weight;
    assert(newContainer.weight >= 0.0f);
    processEdgeForwardSection(segment,
                              brick,
                              newContainer);
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
processPendingEdge(NetworkSegment &segment,
                   Brick &brick,
                   const PendingEdgeContainer &container)
{
    assert(container.sourceSide != 0);
    assert(brick.edges.inUse != 0);

    const uint32_t numberOfEdgeSections = static_cast<uint32_t>(brick.edges.numberOfItems);
    EdgeSection* forwardEnd = getEdgeBlock(brick);
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
            if(edgeSection->sourceId != UNINIT_STATE_32
                    && edgeSection->sourceId != 0)
            {
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
            processEdgeForwardSection(segment,
                                      brick,
                                      newContainer);
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
processLearningEdgeReply(Brick &brick,
                         const LearningEdgeReplyContainer &container,
                         const uint8_t side)
{
    assert(brick.edges.inUse != 0);

    EdgeSection* edgeSections = getEdgeBlock(brick);
    edgeSections[container.sourceEdgeSectionId].edges[side].targetId =
            static_cast<uint32_t>(container.targetEdgeSectionId);
}

//==================================================================================================

/**
 * processing of all incoming messages in a brick
 *
 * @return false if a message-type does not exist, else true
 */
inline bool
processIncomingMessage(NetworkSegment &segment,
                       Brick &brick,
                       const uint8_t side,
                       DataBuffer* message)
{
    bool result = true;

    // get start and end of the message-payload
    uint8_t* data = static_cast<uint8_t*>(message->data);
    uint8_t* end = data + message->bufferPosition;

    while(data < end)
    {
        const uint8_t type = data[0];
        void* obj = static_cast<void*>(data);

        switch(type)
        {
            // -------------------------------------------------------------------------------------
            case STATUS_EDGE_CONTAINER:
            {
                const UpdateEdgeContainer edge = *static_cast<UpdateEdgeContainer*>(obj);
                assert(edge.targetId != UNINIT_STATE_32);
                processUpdateEdge(brick, edge, side);
                data += sizeof(UpdateEdgeContainer);
                break;
            }
            // -------------------------------------------------------------------------------------
            case PENDING_EDGE_CONTAINER:
            {
                const PendingEdgeContainer edge = *static_cast<PendingEdgeContainer*>(obj);
                assert(edge.sourceEdgeSectionId != UNINIT_STATE_32);
                processPendingEdge(segment, brick, edge);
                data += sizeof(PendingEdgeContainer);
                break;
            }
            // -------------------------------------------------------------------------------------
            case FOREWARD_EDGE_CONTAINER:
            {
                const EdgeContainer edge = *static_cast<EdgeContainer*>(obj);
                assert(edge.targetEdgeSectionId != UNINIT_STATE_32);
                processEdgeForwardSection(segment, brick, edge);
                data += sizeof(EdgeContainer);
                break;
            }
            // -------------------------------------------------------------------------------------
            case AXON_EDGE_CONTAINER:
            {
                const AxonEdgeContainer edge = *static_cast<AxonEdgeContainer*>(obj);
                assert(edge.targetAxonId != UNINIT_STATE_32);
                processAxon(segment, brick, edge);
                data += sizeof(AxonEdgeContainer);
                break;
            }
            // -------------------------------------------------------------------------------------
            case LEARNING_EDGE_CONTAINER:
            {
                const LearingEdgeContainer edge = *static_cast<LearingEdgeContainer*>(obj);
                assert(edge.sourceEdgeSectionId != UNINIT_STATE_32);
                assert(side != 0);
                processLerningEdge(segment, brick, edge, side);
                data += sizeof(LearingEdgeContainer);
                break;
            }
            // -------------------------------------------------------------------------------------
            case LEARNING_REPLY_EDGE_CONTAINER:
            {
                const LearningEdgeReplyContainer edge =
                        *static_cast<LearningEdgeReplyContainer*>(obj);
                assert(edge.sourceEdgeSectionId != UNINIT_STATE_32);
                processLearningEdgeReply(brick, edge, side);
                data += sizeof(LearningEdgeReplyContainer);
                break;
            }
            // -------------------------------------------------------------------------------------
            case UNDEFINED_CONTAINER:
            {
                return result;
            }
            // -------------------------------------------------------------------------------------
            default:
                result = false;
        }
    }

    return result;
}

/**
 * @brief ProcessingUnit::processIncomingMessages
 * @param brick
 * @return
 */
inline void
processIncomingMessages(NetworkSegment &segment,
                        Brick &brick)
{
    // process normal communication
    for(uint8_t side = 0; side < 23; side++)
    {
        if(brick.neighbors[side].inUse == 1)
        {
            StackBuffer* currentBuffer = brick.neighbors[side].currentBuffer;
            DataBuffer* currentBlock = Kitsunemimi::getFirstElement_StackBuffer(*currentBuffer);

            while(currentBlock != nullptr)
            {
                processIncomingMessage(segment, brick, side, currentBlock);
                Kitsunemimi::removeFirst_StackBuffer(*currentBuffer);
                currentBlock = Kitsunemimi::getFirstElement_StackBuffer(*currentBuffer);
            }
        }
    }
}

/**
 * @brief processIncomingMessages
 * @param segment
 * @param brick
 * @param side
 */
inline void
processIncomingMessages2(NetworkSegment &segment,
                         Brick &brick,
                         const uint8_t side)
{
    if(brick.neighbors[side].inUse == 1)
    {
        StackBuffer* currentBuffer = brick.neighbors[side].currentBuffer;
        DataBuffer* currentBlock = Kitsunemimi::getFirstElement_StackBuffer(*currentBuffer);

        while(currentBlock != nullptr)
        {
            processIncomingMessage(segment, brick, side, currentBlock);
            Kitsunemimi::removeFirst_StackBuffer(*currentBuffer);
            currentBlock = Kitsunemimi::getFirstElement_StackBuffer(*currentBuffer);
        }
    }
}

} // namespace KyoukoMind

#endif // EDGE_CONTAINER_PROCESSING_H
