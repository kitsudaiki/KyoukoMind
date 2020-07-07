/**
 *  @file    process_update.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#ifndef PROCESS_UPDATE_H
#define PROCESS_UPDATE_H

#include <common.h>
#include <kyouko_root.h>

#include <core/processing/objects/edges.h>
#include <core/processing/objects/container_definitions.h>
#include <core/processing/methods/synapse_methods.h>
#include <core/methods/data_connection_methods.h>

#include <core/brick.h>
#include <core/network_segment.h>

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
    const float diff = edgeSection.edges[inititalSide].weight - newValue;
    edgeSection.edges[inititalSide].weight = newValue;

    if(edgeSection.sourceId != UNINIT_STATE_32)
    {
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

}

#endif // PROCESS_UPDATE_H
