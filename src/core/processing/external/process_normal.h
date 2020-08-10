/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef PROCESS_NORMAL_H
#define PROCESS_NORMAL_H

#include <common.h>
#include <kyouko_root.h>

//==================================================================================================

/**
 * processEdgeForwardSection
 *
 * @param forwardEdgeSectionId target edge-section of the incoming edge
 * @param weight incoming edge-weight
 */
/*inline void
processEdgeForwardSection(NetworkSegment &segment,
                          Brick &brick,
                          const EdgeContainer &container)
{
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
        if(tempEdge.weight == 0.0f) {
            continue;
        }

        if(side == 22)
        {
            if(tempEdge.targetId == UNINIT_STATE_32){
                continue;
            }

            void* transferData = segment.synapseEdges.buffer.data;
            SynapseTransfer* synapseTransfer = static_cast<SynapseTransfer*>(transferData);
            const uint32_t pos = segment.getNextTransferPos();
            synapseTransfer[pos].weight = tempEdge.weight * ratio;
            synapseTransfer[pos].sourceEdgeId = container.targetEdgeSectionId;
            synapseTransfer[pos].brickId = brick.brickId;
        }
        else
        {
            if(tempEdge.targetId != UNINIT_STATE_32)
            {
                // normal external edge
                EdgeContainer newContainer;
                newContainer.targetEdgeSectionId = tempEdge.targetId;
                newContainer.weight = tempEdge.weight * ratio;
                Kitsunemimi::addObject_StackBuffer(*brick.neighbors[side].outgoingBuffer,
                                                   &newContainer);
            }
            else
            {
                // send pendinge-edge if the learning-step is not finished
                PendingEdgeContainer newContainer;
                newContainer.weight = tempEdge.weight * ratio;
                newContainer.sourceEdgeSectionId = container.targetEdgeSectionId;
                newContainer.sourceSide = 23 - side;
                Kitsunemimi::addObject_StackBuffer(*brick.neighbors[side].outgoingBuffer,
                                                   &newContainer);
            }
        }
    }
}*/

//==================================================================================================

/**
 * process learning-edge to create a new forward-edge
 *
 * @param sourceEdgeSectionId id of the source-edge-section within the last brick
 * @param weight weight of the new edge
 * @param initSide side of the incoming message
 */
/*/*inline void
processLearningEdge(NetworkSegment &segment,
                    Brick &brick,
                    const LearingEdgeContainer &container,
                    const uint8_t initSide)
{
    const uint64_t targetEdgeId = brick. addEmptyEdgeSection(initSide,
                                                             container.sourceEdgeSectionId);

    // create reply-message
    LearningEdgeReplyContainer reply;
    reply.sourceEdgeSectionId = container.sourceEdgeSectionId;
    reply.targetEdgeSectionId = static_cast<uint32_t>(targetEdgeId);
    Kitsunemimi::addObject_StackBuffer(*brick.neighbors[initSide].outgoingBuffer,
                                       &reply);

    // process the content of the edge
    EdgeContainer newContainer;
    newContainer.targetEdgeSectionId = static_cast<uint32_t>(targetEdgeId);
    newContainer.weight = container.weight;
    processEdgeForwardSection(segment,
                              brick,
                              newContainer);
}*/

//==================================================================================================

/**
 * process axon
 *
 * @param targetId section-target-id at the end of the axon
 * @param path rest of the path of the axon
 * @param weight weight of the axon
 */
/*inline void
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
}*/

//==================================================================================================

/**
 * process pending-edge in the cycle after the learning-edge
 *
 * @param sourceId source-section-id of the pending-edge
 * @param sourceSide side of the incoming pending-edge
 * @param weight weight of the pending-edge
 */
/*inline void
processPendingEdge(NetworkSegment &segment,
                   Brick &brick,
                   const PendingEdgeContainer &container)
{
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
        // if found, then process the pending-edge als normal forward-edge
        if(edgeSection->status == ACTIVE_SECTION
                && container.sourceEdgeSectionId == edgeSection->sourceId
                && container.sourceSide == edgeSection->sourceSide)
        {
            EdgeContainer newContainer;
            newContainer.targetEdgeSectionId = forwardEdgeSectionId;
            newContainer.weight = container.weight;
            processEdgeForwardSection(segment,
                                      brick,
                                      newContainer);
        }

        forwardEdgeSectionId--;
    }
}*/

#endif // PROCESS_NORMAL_H
