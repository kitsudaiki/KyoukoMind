/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef EDGE_PROCESSING_H
#define EDGE_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/edges.h>
#include <core/objects/transfer_objects.h>
#include <core/objects/brick.h>
#include <core/objects/segment.h>
#include <core/objects/item_buffer.h>
#include <core/objects/synapses.h>
#include <core/objects/global_values.h>

#include <core/connection_handler/client_connection_handler.h>


/**
 * @brief createSynapse
 * @param section
 * @param edge
 * @param weight
 */
inline void
createSynapseSection(EdgeSection &section,
                     EdgeGroup &edgeGroup,
                     Brick* brick)
{
    if(brick->isInputBrick == false
            && edgeGroup.synapseSectionId == UNINIT_STATE_32  // no synapse already exist
            && brick->nodeBrickId != UNINIT_STATE_32  // current brick must be a node-brick
            && section.numberOfUsedSynapseSections < 32)
    {
        brick->synapseCreateActivity++;
        section.numberOfUsedSynapseSections++;

        SynapseSection newSection;
        const uint64_t newPos = KyoukoRoot::m_segment->synapses.addNewItem(newSection);
        if(newPos == UNINIT_STATE_64) {
            return;
        }

        edgeGroup.synapseSectionId = static_cast<uint32_t>(newPos);
        // very left bit says if this connection is new
        edgeGroup.synapseSectionId = edgeGroup.synapseSectionId | 0x80000000;
    }

    edgeGroup.synapseWeight = 0.0f;
}

/**
 * @brief processSynapseConnection
 * @param edge
 * @param weight
 * @param pos
 * @param edgeSectionPos
 * @return
 */
inline void
processSynapseConnection(EdgeGroup &edgeGroup,
                         const float weight,
                         const uint16_t positionInEdge,
                         const uint32_t edgeSectionPos,
                         Brick* brick)
{
    if(edgeGroup.synapseSectionId != UNINIT_STATE_32)
    {
        //return usedWeight;
        SynapseTransfer newTransfer;
        newTransfer.synapseSectionId = edgeGroup.synapseSectionId & 0x7FFFFFFF;
        newTransfer.isNew = edgeGroup.synapseSectionId >> 31;
        newTransfer.weight = weight;
        newTransfer.brickId = brick->brickId;
        newTransfer.nodeBrickId = brick->nodeBrickId;
        newTransfer.positionInEdge = static_cast<uint8_t>(positionInEdge);
        newTransfer.sourceEdgeId = edgeSectionPos;

        // set the left bit to 0 to mark this connections as not new anymore
        edgeGroup.synapseSectionId = edgeGroup.synapseSectionId & 0x7FFFFFFF;

        KyoukoRoot::m_segment->synapseTransfers.addNewItem(newTransfer);
        brick->synapseActivity++;

        return;
    }
}

/**
 * @brief initBrick
 * @param section
 * @param edge
 * @param lastLocation
 */
inline bool
initBrick(EdgeSection &section,
          EdgeGroup &edgeGroup,
          const uint32_t lastLocation)
{
    Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(lastLocation)];
    edgeGroup.brickLocation = brick->getRandomNeighbor(lastLocation);
    if(getBrickId(edgeGroup.brickLocation) == UNINIT_STATE_24) {
        return false;
    }
    brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(edgeGroup.brickLocation)];
    brick->edgeCreateActivity++;

    createSynapseSection(section, edgeGroup, brick);

    return true;
}

/**
 * @brief processEdgeGroup
 * @param section
 * @param groupPos
 */
inline float
expandEdgeGroup(EdgeSection &section,
                const uint16_t groupPos,
                const float weight)
{
    EdgeGroup* group = &section.edgeGroups[groupPos];
    Edge* edges = group->edges;
    const uint32_t freeEdges = group->getNumberOfUninit();
    if(freeEdges == 0) {
        return 0.0f;
    }

    const float part = weight / static_cast<float>(freeEdges);

    for(uint16_t i = 0; i < 4; i++)
    {
        if(edges[i].nextGroup == UNINIT_STATE_16)
        {
            edges->nextGroup = section.getFreePosition();
            edges->edgeWeight = part;

            EdgeGroup* next = &section.edgeGroups[edges->nextGroup];
            next->prevGroup = groupPos;
            next->prevGroupPos = i;
        }
    }

    return weight;
}

/**
 * @brief nextEdgeSectionStep
 * @param section
 * @param pos
 * @param weight
 * @param lastBrickId
 * @return
 */
inline void
processEdgeGroup(EdgeSection &section,
                 const uint16_t groupPos,
                 float weight,
                 const uint32_t lastLocation,
                 const uint32_t edgeSectionPos)
{
    // end-condition
    if(weight < 0.1f) {
        return;
    }

    EdgeGroup* group = &section.edgeGroups[groupPos];

    // init brick if necessary
    if(getBrickId(*group) == UNINIT_STATE_24)
    {
        if(initBrick(section, *group, lastLocation) == false) {
            return;
        }

        if(group->synapseSectionId != UNINIT_STATE_32)
        {
            // add weight to synapse-section
            uint32_t* randValues = getBuffer<uint32_t>(KyoukoRoot::m_segment->randomIntValues);
            section.randomPos = (section.randomPos + 1) % 1024;
            const float randValue = static_cast<float>(randValues[section.randomPos] % 1024) / 1024.0f;
            group->synapseWeight += weight * randValue;
        }
    }

    // update group weight
    float totalWeight = group->count();
    if(weight > totalWeight) {
        totalWeight += expandEdgeGroup(section, groupPos, weight - totalWeight);
    }

    Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(*group)];

    if(weight > group->synapseWeight)
    {
        processSynapseConnection(*group,
                                 group->synapseWeight,
                                 groupPos,
                                 edgeSectionPos,
                                 brick);
        weight -= group->synapseWeight;
    }
    else
    {
        processSynapseConnection(*group,
                                 weight,
                                 groupPos,
                                 edgeSectionPos,
                                 brick);
        weight = 0.0f;
        return;
    }

    for(uint32_t i = 0; i < 4; i++)
    {
        const Edge currentEdge = group->edges[i];

        // process connection to synapse
        if(weight > currentEdge.edgeWeight)
        {
            processEdgeGroup(section,
                             currentEdge.nextGroup,
                             currentEdge.edgeWeight,
                             group->brickLocation,
                             edgeSectionPos);
            weight -= currentEdge.edgeWeight;
        }
        else
        {
            processEdgeGroup(section,
                             currentEdge.nextGroup,
                             weight,
                             group->brickLocation,
                             edgeSectionPos);
            weight = 0.0f;
            return;
        }
    }
}

/**
 * @brief cleanupEdgeSection
 * @param section
 */
inline void
cleanupEdgeSection(EdgeSection &section,
                   const uint16_t groupPos)
{
    EdgeGroup* group = &section.edgeGroups[groupPos];
    group->squash();

    if(groupPos == 0) {
        return;
    }

    Edge* parentEdge = &section.edgeGroups[group->prevGroup].edges[group->prevGroupPos];

    const float totalWeight = group->count();
    if(totalWeight == 0.0f)
    {
        group->reset();
        parentEdge->nextGroup = UNINIT_STATE_16;
        parentEdge->edgeWeight = 0.0f;
    }
    else
    {
        parentEdge->edgeWeight = totalWeight;
    }

    cleanupEdgeSection(section, group->prevGroup);

    return;
}

/**
 * @brief processEdgeSection
 * @return
 */
inline uint32_t
processEdgeSection()
{
    uint32_t count = 0;

    // prepare pointer
    Segment* segment = KyoukoRoot::m_segment;
    EdgeSection* edgeSections = getBuffer<EdgeSection>(segment->edges);
    AxonTransfer* axonTransfers = getBuffer<AxonTransfer>(segment->axonTransfers);
    Brick* bricks = getBuffer<Brick>(segment->bricks);
    GlobalValues* globalValues = getBuffer<GlobalValues>(segment->globalValues);

    // insert input-values from brick
    const std::vector<float> inputValues = bricks[1].getInputValues();
    for(uint32_t i = 0; i < inputValues.size(); i++)
    {
        if(inputValues.at(i) == 1.0f) {
            axonTransfers[i].weight = globalValues->actionPotential;
        } else {
            axonTransfers[i].weight = 0.0f;
        }
    }

    // process axon-messages
    for(uint32_t i = 0; i < segment->axonTransfers.itemCapacity; i++)
    {
        const AxonTransfer* currentTransfer = &axonTransfers[i];
        if(currentTransfer->weight == 0.0f) {
            continue;
        }

        EdgeSection* currentSection = &edgeSections[i];
        Brick* brick = &bricks[currentTransfer->brickId];
        brick->nodeActivity++;

        //if(brick->isOutputBrick == false)
        if(currentTransfer->brickId != 60)
        {
            processEdgeGroup(*currentSection,
                             0,
                             currentTransfer->weight,
                             currentSection->targetBrickId,
                             i);
        }
        else
        {
            const uint32_t offset = brick->nodePos;
            const uint32_t max = brick->getNumberOfOutputValues();
            if(max > 0) {
                brick->setOutputValue((i - offset) % max, currentTransfer->weight);
            }
        }

        // increase counter for monitoring-output
        count++;
    }

    return count;
}


/**
 * @brief updateEdgeSection
 * @return
 */
inline uint32_t
updateEdgeSection()
{
    uint32_t count = 0;

    // prepare pointer
    Segment* segment = KyoukoRoot::m_segment;
    EdgeSection* edgeSections = getBuffer<EdgeSection>(segment->edges);

    // prepare border for iterator
    UpdateTransfer* start = getBuffer<UpdateTransfer>(segment->updateTransfers);
    UpdateTransfer* end = start + segment->updateTransfers.itemCapacity;

    for(const UpdateTransfer* container = start;
        container < end;
        container++)
    {
        // skip containers without a target
        if(container->targetId == UNINIT_STATE_32) {
            continue;
        }

        // iterade over all update-containers coming from the gpu
        EdgeSection* section = &edgeSections[container->targetId];
        EdgeGroup* edgeGroup = &section->edgeGroups[container->positionInEdge];
        Edge* prevEdge = &section->edgeGroups[edgeGroup->prevGroup].edges[edgeGroup->prevGroupPos];

        if(container->deleteEdge > 0)
        {
            KyoukoRoot::m_segment->synapses.deleteDynamicItem(edgeGroup->synapseSectionId);
            Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(*edgeGroup)];
            brick->synapseDeleteActivity++;
            const int16_t numSyn = section->numberOfUsedSynapseSections;
            section->numberOfUsedSynapseSections = (numSyn < 0) * 0 + (numSyn >= 0) * numSyn;

            section->numberOfUsedSynapseSections--;
            edgeGroup->synapseSectionId = UNINIT_STATE_32;
            edgeGroup->synapseWeight = 0;

            prevEdge->edgeWeight = edgeGroup->synapseWeight;
        }
        else
        {
            edgeGroup->synapseWeight = container->newWeight;
            prevEdge->edgeWeight = edgeGroup->synapseWeight;
        }

        // increase counter for monitoring-output
        cleanupEdgeSection(*section, container->positionInEdge);
        count++;
    }

    return count;
}

inline void
clearAllEdgeSections()
{
    // prepare pointer
    Segment* segment = KyoukoRoot::m_segment;
    EdgeSection* edgeSections = getBuffer<EdgeSection>(segment->edges);

    // TODO
}

#endif // EDGE_PROCESSING_H

