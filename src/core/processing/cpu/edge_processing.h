/**
 * @file        kyouko_root.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
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
 * @brief initBrick
 * @param section
 * @param edge
 * @param lastLocation
 */
inline uint16_t
createNewEdge(EdgeSection &section,
              const float weight,
              Brick* sourceBrick)
{
    // get random brick as target
    uint32_t* randValues = getBuffer<uint32_t>(KyoukoRoot::m_segment->randomIntValues);
    section.randomPos = (section.randomPos + 1) % 1024;
    const uint32_t pos = randValues[section.randomPos] % KyoukoRoot::m_segment->numberOfNodeBricks;
    Brick* targetBrick = KyoukoRoot::m_segment->nodeBricks[pos];

    // input-brick are never allowed to directly connect to the output-brick
    if(sourceBrick->isInputBrick
            && targetBrick->isOutputBrick)
    {
        return UNINIT_STATE_16;
    }

    if(targetBrick->isInputBrick) {
        return UNINIT_STATE_16;
    }

    // init new synapse-section and register it inside the edge
    if(targetBrick->isInputBrick == false
            && section.numberOfUsedSynapseSections < 32)
    {
        // try to place a new empty section in the buffer
        SynapseSection newSection;
        const uint64_t newPos = KyoukoRoot::m_segment->synapses.addNewItem(newSection);
        if(newPos == UNINIT_STATE_64) {
            return UNINIT_STATE_16;
        }

        // update counter for monitoring
        targetBrick->synapseCreateActivity++;
        targetBrick->edgeCreateActivity++;
        section.numberOfUsedSynapseSections++;

        // create new edge
        Edge newEdge;
        newEdge.synapseSectionId = static_cast<uint32_t>(newPos);
        newEdge.synapseSectionId = newEdge.synapseSectionId | 0x80000000;
        newEdge.synapseWeight = 0.0f;
        newEdge.brickId = targetBrick->brickId;
        newEdge.synapseWeight = weight;

        return section.append(newEdge);
    }

    return UNINIT_STATE_16;
}

/**
 * @brief processSynapseConnection
 * @param edge
 * @param weight
 * @param positionInSection
 * @param edgeSectionPos
 */
inline void
processSynapseConnection(Edge &edge,
                         const float weight,
                         const uint8_t positionInSection,
                         const uint32_t edgeSectionPos)
{
    // prepare
    Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[edge.brickId];
    assert(brick->nodeBrickId != UNINIT_STATE_32);

    //return usedWeight;
    SynapseTransfer newTransfer;
    newTransfer.synapseSectionId = edge.synapseSectionId & 0x7FFFFFFF;
    newTransfer.isNew = edge.synapseSectionId >> 31;
    newTransfer.weight = weight;
    newTransfer.brickId = brick->brickId;
    newTransfer.nodeBrickId = brick->nodeBrickId;
    newTransfer.positionInEdge = positionInSection;
    newTransfer.sourceEdgeId = edgeSectionPos;

    KyoukoRoot::m_segment->synapseTransfers.addNewItem(newTransfer);
    brick->synapseActivity++;

    // set the left bit to 0 to mark this connections as not new anymore
    edge.synapseSectionId = edge.synapseSectionId & 0x7FFFFFFF;
}

/**
 * @brief processEdgeGroup
 * @param section
 * @param weight
 * @param brickId
 * @param edgeSectionPos
 * @param sourceBrick
 */
inline void
processEdgeGroup(EdgeSection &section,
                 float weight,
                 const uint32_t edgeSectionPos,
                 Brick* sourceBrick)
{
    // prepare
    Edge* currentEdge = &section.edges[0];
    uint8_t currentPos = 0;

    // init learning
    float toLearn = weight - section.getTotalWeight();
    toLearn = (toLearn < 0.0f) * 0.0f + (toLearn >= 0.0f) * toLearn;

    // iterate over the linked list of edges
    while(currentEdge->next != UNINIT_STATE_16
          && weight > 0.0f)
    {
        currentPos = static_cast<uint8_t>(currentEdge->next);
        currentEdge = &section.edges[currentEdge->next];
        assert(currentEdge->synapseSectionId != UNINIT_STATE_32);

        // share learning-weight
        const float diff = toLearn * (1.0f - currentEdge->hardening);
        currentEdge->synapseWeight += diff;
        toLearn -= diff;

        // trigger synapse
        const float newWeight = (weight > currentEdge->synapseWeight) * currentEdge->synapseWeight
                                + (weight <= currentEdge->synapseWeight) * weight;
        processSynapseConnection(*currentEdge, newWeight, currentPos, edgeSectionPos);
        weight -= newWeight;
    }

    // if not everything of the new weight was shared, then create a new edge for the remaining
    if(toLearn > 2.0f)
    {
        // try to create new edge
        const uint16_t pos = createNewEdge(section, toLearn, sourceBrick);
        if(pos == UNINIT_STATE_16) {
            return;
        }

        // process the new created edge
        processSynapseConnection(section.edges[pos],
                                 toLearn,
                                 pos,
                                 edgeSectionPos);
    }
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

    // process axon-messages
    for(uint32_t i = 0; i < segment->axonTransfers.itemCapacity; i++)
    {
        // precheck transfer-message
        const AxonTransfer* currentTransfer = &axonTransfers[i];
        if(currentTransfer->weight == 0.0f) {
            continue;
        }

        // update counter
        Brick* sourceBrick = &bricks[currentTransfer->brickId];
        sourceBrick->nodeActivity++;
        count++;

        // harden section
        if(globalValues->lerningValue != 0.0f)
        {
            EdgeSection* currentSection = &edgeSections[i];
            currentSection->harden(globalValues->lerningValue);
        }


        // skip output-bricks, because the are handle in another stage
        if(sourceBrick->isOutputBrick) {
            continue;
        }

        // process axon-transfer-message
        EdgeSection* currentSection = &edgeSections[i];
        processEdgeGroup(*currentSection,
                         currentTransfer->weight,
                         i,
                         sourceBrick);

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

    // iterade over all update-containers coming from the gpu
    for(const UpdateTransfer* container = start;
        container < end;
        container++)
    {
        // skip containers without a target
        if(container->targetId == UNINIT_STATE_32) {
            continue;
        }

        // prepare pointer
        EdgeSection* section = &edgeSections[container->targetId];
        Edge* edge = &section->edges[container->positionInEdge];
        assert(edge->brickId != UNINIT_STATE_32);

        if(container->deleteEdge > 0)
        {
            KyoukoRoot::m_segment->synapses.deleteDynamicItem(edge->synapseSectionId);

            // update counter
            Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[edge->brickId];
            brick->synapseDeleteActivity++;
            brick->edgeDeleteActivity++;
            section->numberOfUsedSynapseSections--;

            section->remove(container->positionInEdge);
        }
        else
        {
            edge->synapseWeight = container->newWeight;
            //edge->hardening = container->hardening;
        }

        // increase counter for monitoring-output
        count++;
    }

    return count;
}

#endif // EDGE_PROCESSING_H

