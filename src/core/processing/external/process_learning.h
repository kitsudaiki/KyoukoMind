/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef PROCESS_LEARNING_H
#define PROCESS_LEARNING_H

#include <common.h>
#include <kyouko_root.h>


//==================================================================================================

/**
 * @brief initializeNewEdge
 * @param segment
 * @param brick
 * @param edgeSection
 * @param edgeSectionId
 * @param side
 * @param weight
 */
/*inline void
initializeNewEdge(NetworkSegment &segment,
                  Brick &brick,
                  EdgeSection* edgeSection,
                  const uint32_t edgeSectionId,
                  const uint8_t side,
                  const float weight)
{
    if(side == 22)
    {
        const uint64_t id = segment.addEmptySynapseSection(edgeSectionId, brick.brickId);
        const uint32_t targetId = static_cast<uint32_t>(id);
        SynapseSection* synapseSection = &segment.getSynapseSectionBlock()[targetId];
        synapseSection->sourceEdgeId = edgeSectionId;
        edgeSection->edges[22].targetId = targetId;
        edgeSection->edges[22].weight = weight;
    }
    else
    {
        // send new learning-edge
        LearingEdgeContainer newContainer;
        newContainer.sourceEdgeSectionId = edgeSectionId;
        newContainer.weight = weight;
        Kitsunemimi::addObject_StackBuffer(*brick.neighbors[side].outgoingBuffer,
                                           &newContainer);
    }
}*/

//==================================================================================================

/**
 * @brief singleLearnStep
 * @param segment
 * @param brick
 * @param edgeSection
 * @param edgeSectionId
 * @param weight
 * @return
 */
/*inline float
singleLearnStep(NetworkSegment &segment,
                Brick &brick,
                EdgeSection* edgeSection,
                const uint32_t edgeSectionId,
                const float weight)
{
    // TODO: offset +9 only for now
    const uint8_t side = (brick.randValue[brick.randValuePos] % 14) + 9;
    brick.randValuePos = (brick.randValuePos + 1) % 1024;

    // only process available edges
    if(edgeSection->edges[side].available == 0) {
        return 0.0f;
    }

    // get rand weight-value
    const float currentSideWeight = brick.randWeight[brick.randWeightPos] * weight;
    brick.randWeightPos = (brick.randWeightPos + 1) % 999;

    // update values of section
    edgeSection->edges[side].weight += currentSideWeight;

    uint32_t targetId = edgeSection->edges[side].targetId;
    if(targetId == UNINIT_STATE_32) {
        initializeNewEdge(segment, brick, edgeSection, edgeSectionId, side, currentSideWeight);
    }

    return currentSideWeight;
}*/

//==================================================================================================

/**
 * create new edges for the current section
 *
 * @param currentSection pointer to the current section
 * @param forwardEdgeSectionId the id of the current section
 * @param weight weight with have to be consumed from the updated edges
 */
/*inline void
learningEdgeSection(NetworkSegment &segment,
                    Brick &brick,
                    EdgeSection* edgeSection,
                    const uint32_t edgeSectionId,
                    const float weight)
{
    if(weight < 0.5f) {
        return;
    }

    // try to create up to three new edges
    edgeSection->totalWeight += singleLearnStep(segment, brick, edgeSection, edgeSectionId, weight);
    edgeSection->totalWeight += singleLearnStep(segment, brick, edgeSection, edgeSectionId, weight);
    edgeSection->totalWeight += singleLearnStep(segment, brick, edgeSection, edgeSectionId, weight);
}*/

//==================================================================================================

/**
 * @brief processDirectEdge
 * @param brick
 * @param edge
 */
/*inline void
processLearningEdgeReply(Brick &brick,
                         const LearningEdgeReplyContainer &container,
                         const uint8_t side)
{
    EdgeSection* edgeSections = getEdgeBlock(brick);
    edgeSections[container.sourceEdgeSectionId].edges[side].targetId =
            static_cast<uint32_t>(container.targetEdgeSectionId);
}*/

#endif // PROCESS_LEARNING_H
