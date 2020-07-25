/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef EDGE_PROCESSING_H
#define EDGE_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/processing/internal/objects/edges.h>
#include <core/processing/internal/objects/transfer_objects.h>

#include <core/object_handling/brick.h>
#include <core/object_handling/segment.h>
#include <core/object_handling/item_buffer.h>

#include <core/processing/internal/objects/synapses.h>

namespace KyoukoMind
{

/**
 * @brief lernEdge
 * @param edge
 * @param weight
 */
inline void
lernEdge(EdgeSection &section,
         Edge &edge,
         const uint16_t pos,
         const float weight)
{
    uint32_t* randValues = getBuffer<uint32_t>(KyoukoRoot::m_segment->randomIntValues);

    // try to create new synapse-section
    Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(edge.location)];
    if(edge.synapseSectionId == UNINIT_STATE_32
            && brick->isNodeBrick)
    {
        SynapseSection newSection;
        const uint64_t newPos = KyoukoRoot::m_segment->synapses.addNewItem(newSection);
        assert(newPos != UNINIT_STATE_64);
        edge.synapseSectionId = static_cast<uint32_t>(newPos);
        // very left bit says if this connection is new
        edge.synapseSectionId = edge.synapseSectionId | 0x80000000;
    }

    // update weight in current edge

    if(pos < 128)
    {
        float edgeWeight = weight;

        if(edge.synapseSectionId != UNINIT_STATE_32)
        {
            section.randomPos = (section.randomPos + 1) % 1024;
            const float randRatio = static_cast<float>(randValues[section.randomPos] % 1024) / 1024.0f;
            edgeWeight = weight * randRatio;
            edge.synapseWeight += weight * (1.0f - randRatio);
        }

        // update weight in next edges
        section.randomPos = (section.randomPos + 1) % 1024;
        const float randRatio = static_cast<float>(randValues[section.randomPos] % 1024) / 1024.0f;
        section.edges[pos * 2].edgeWeight += edgeWeight * randRatio;
        section.edges[(pos * 2) + 1].edgeWeight += edgeWeight * (1.0f - randRatio);
    }
    else
    {
        if(edge.synapseSectionId != UNINIT_STATE_32) {
            edge.synapseWeight = edge.edgeWeight;
        }
    }

    //std::cout<<"+++ learn update: "<<(weight * randValue)<<std::endl;

    return;
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
processSynapseConnection(Edge &edge,
                         const float ratio,
                         const uint16_t pos,
                         const uint32_t edgeSectionPos)
{
    if(edge.synapseSectionId != UNINIT_STATE_32)
    {
        //return usedWeight;
        SynapseTransfer newTransfer;
        newTransfer.synapseSectionId = edge.synapseSectionId & 0x7FFFFFFF;
        newTransfer.isNew = edge.synapseSectionId >> 31;
        if(newTransfer.isNew != 0
                && edgeSectionPos == 0) {
            assert(newTransfer.isNew == 1);
            // std::cout<<"create pos: "<<(int)pos<<"    edge-section-id: "<<edgeSectionPos<<"   synapse-section-id: "<<newTransfer.synapseSectionId<<std::endl;
        }
        newTransfer.weight = edge.synapseWeight * ratio;
        newTransfer.brickId = getBrickId(edge.location);
        newTransfer.positionInEdge = static_cast<uint8_t>(pos);
        newTransfer.sourceEdgeId = edgeSectionPos;

        // set the left bit to 0 to mark this connections as not new anymore
        edge.synapseSectionId = edge.synapseSectionId & 0x7FFFFFFF;
        assert(edge.synapseSectionId >> 31 == 0);

        KyoukoRoot::m_segment->synapseTransfers.addNewItem(newTransfer);
        return;
    }
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
nextEdgeSectionStep(EdgeSection &section,
                    const uint16_t pos,
                    const float weight,
                    const uint32_t lastLocation,
                    const uint32_t edgeSectionPos)
{
    assert(getBrickId(lastLocation) != UNINIT_STATE_24);
    // end-condition
    if(weight < 0.1f) {
        return;
    }

    // get initial values
    Edge* edge = &section.edges[pos];

    // init new edge, if necessary
    if(getBrickId(edge->location) == UNINIT_STATE_24)
    {
        Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(lastLocation)];
        edge->location = brick->getRandomNeighbor(lastLocation);
    }

    float ratio = 0.0f;

    if(getBrickId(edge->location) != UNINIT_STATE_24)
    {
        if(pos < 128)
        {
            // calculate and process ratio
            const float totalWeight = edge->synapseWeight + 0.0000001f
                                      + section.edges[pos * 2].edgeWeight
                                      + section.edges[(pos * 2) + 1].edgeWeight;

            ratio = weight / totalWeight;
            if(ratio > 1.0f) {
                lernEdge(section, *edge, pos, weight - totalWeight);
            }

            // limit ratio to 1.0
            ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;

            // update remaining weight based on the ratio
            nextEdgeSectionStep(section,
                                pos * 2,
                                section.edges[pos * 2].edgeWeight * ratio,
                                edge->location,
                                edgeSectionPos);

            nextEdgeSectionStep(section,
                                (pos * 2) + 1,
                                section.edges[(pos * 2) + 1].edgeWeight * ratio,
                                edge->location,
                                edgeSectionPos);
        }
        else
        {
            // calculate and process ratio
            const float totalWeight = edge->synapseWeight + 0.0000001f;
            ratio = weight / totalWeight;
            if(ratio > 1.0f) {
                lernEdge(section, *edge, pos, weight - totalWeight);
            }

            // limit ratio to 1.0
            ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;
        }

        // process connection to synapse
        processSynapseConnection(*edge, ratio, pos, edgeSectionPos);
    }

    return;
}

/**
 * @brief cleanupEdgeSection
 * @param section
 */
inline void
cleanupEdgeSection(EdgeSection &section)
{
    for(uint32_t i = 255; i >= 128; i--)
    {
        Edge* edge = &section.edges[i];
        if(edge->synapseSectionId == UNINIT_STATE_32)
        {
            edge->edgeWeight = 0.0f;
            edge->location = UNINIT_STATE_32;
        }
        else
        {
            edge->edgeWeight = edge->synapseWeight;
        }
    }

    for(uint32_t i = 127; i >= 1; i--)
    {
        Edge* edge = &section.edges[i];
        Edge* next1 = &section.edges[i * 2];
        Edge* next2 = &section.edges[(i * 2) +1];

        if(edge->synapseSectionId == UNINIT_STATE_32
                && next1->edgeWeight == 0.0f
                && next2->edgeWeight == 0.0f)
        {
            edge->edgeWeight = 0.0f;
            edge->location = UNINIT_STATE_32;
            edge->synapseWeight = 0.0f;
        }
        else
        {
            edge->edgeWeight = next1->edgeWeight + next2->edgeWeight + edge->synapseWeight;
        }
    }
}

/**
 * @brief processEdgeSection
 * @param section
 * @param weight
 */
inline uint32_t
processEdgeSection()
{
    uint32_t count = 0;

    Segment* segment = KyoukoRoot::m_segment;
    EdgeSection* edgeSections = getBuffer<EdgeSection>(segment->edges);
    AxonTransfer* axonTransfers = getBuffer<AxonTransfer>(segment->axonTransfers);

    for(uint32_t i = 0; i < segment->axonTransfers.itemCapacity; i++)
    {
        if(axonTransfers[i].weight == 0.0f) {
            continue;
        }

        count++;
        nextEdgeSectionStep(edgeSections[i],
                            1,
                            axonTransfers[i].weight,
                            edgeSections[i].targetBrickId,
                            i);
        cleanupEdgeSection(edgeSections[i]);
    }

    return count;
}


/**
 * @brief updateEdgeSection
 * @param section
 * @param posInSection
 * @param weightDiff
 * @param deleteEdge
 */
inline uint32_t
updateEdgeSection()
{
    uint32_t count = 0;

    Segment* segment = KyoukoRoot::m_segment;
    EdgeSection* edgeSections = getBuffer<EdgeSection>(segment->edges);

    UpdateTransfer* start = getBuffer<UpdateTransfer>(segment->updateTransfers);
    UpdateTransfer* end = start + segment->updateTransfers.itemCapacity;

    for(UpdateTransfer* container = start;
        container < end;
        container++)
    {
        if(container->newWeight < 0.0f) {
            continue;
        }

        count++;

        Edge* edge = &edgeSections[container->targetId].edges[container->positionInEdge];

        // process synapse-connection
        if(container->deleteEdge != 0)
        {
            assert(KyoukoRoot::m_segment->synapses.deleteDynamicItem(edge->synapseSectionId));
            edge->synapseWeight = 0;
            edge->synapseSectionId = UNINIT_STATE_32;
        }
        else
        {
            edge->synapseWeight = container->newWeight;
        }
    }

    return count;
}

}

#endif // EDGE_PROCESSING_H
