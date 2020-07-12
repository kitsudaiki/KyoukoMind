/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef EDGE_PROCESSING_H
#define EDGE_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/processing/objects/edges.h>
#include <core/processing/objects/transfer_objects.h>

#include <core/object_handling/brick.h>
#include <core/object_handling/network_segment.h>
#include <core/object_handling/item_buffer.h>

#include <core/processing/objects/synapses.h>

namespace KyoukoMind
{

/**
 * @brief lernEdge
 * @param edge
 * @param weight
 */
inline void
lernEdge(Edge &edge,
         const float weight)
{
    if(weight < 0.01f) {
        return;
    }

    // get random-values
    const uint32_t randPos = (edge.brickId + edge.lastBrickId * 2) % 1024;
    uint32_t* randValues = getBuffer<uint32_t>(KyoukoRoot::m_segment->randomIntValues);

    // try to create new synapse-section
    if(edge.targetId == UNINIT_STATE_32
            && randValues[(randPos + 1) % 1024] % 3 == 0)
    {
        SynapseSection newSection;
        const uint64_t newPos = KyoukoRoot::m_segment->synapses.addNewItem(newSection);
        assert(newPos != UNINIT_STATE_64);
        edge.targetId = static_cast<uint32_t>(newPos);
    }

    if(edge.targetId == UNINIT_STATE_32)
    {
        edge.weight = 0.0f;
        return;
    }

    // update weight
    const float randValue = static_cast<float>(randValues[(randPos + 1) % 1024] % 1024) / 1024.0f;
    edge.weight += weight * randValue;

    return;
}

/**
 * @brief processEdge
 * @param edge
 * @param weight
 * @param lastBrickId
 * @return
 */
inline float
processEdge(Edge &edge,
            const float weight,
            const uint32_t lastBrickId,
            const uint16_t pos,
            const uint32_t edgeSectionPos)
{
    if(weight < 0.01f) {
        return 0.0f;
    }

    // set brick-ids, if not already done
    if(edge.brickId == UNINIT_STATE_32)
    {
        Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[lastBrickId];
        edge.brickId = brick->getRandomNeighbor(UNINIT_STATE_32);
        edge.lastBrickId = lastBrickId;
    }

    float ratio = weight / edge.weight;
    if(ratio > 1.0f) {
        lernEdge(edge, weight - edge.weight);
    }
    ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;

    const float usedWeight = edge.weight * ratio;
    if(usedWeight >= 0.01f)
    {
        SynapseTransfer newTransfer;
        newTransfer.weight = usedWeight;
        newTransfer.brickId = edge.brickId;
        newTransfer.positionInEdge = static_cast<uint8_t>(pos);
        newTransfer.sourceEdgeId = edgeSectionPos;

        KyoukoRoot::m_segment->synapseTransfers.addNewItem(newTransfer);
        return usedWeight;
    }

    return 0.0f;
}

/**
 * @brief nextEdgeSectionStep
 * @param section
 * @param pos
 * @param weight
 * @param lastBrickId
 * @return
 */
inline float
nextEdgeSectionStep(EdgeSection &section,
                    const uint16_t pos,
                    const float weight,
                    const uint32_t lastBrickId,
                    const uint32_t edgeSectionPos)
{
    if(pos > 255
            || weight < 0.01f)
    {
        return 0.0f;
    }

    float currentWeight = weight;

    currentWeight -= processEdge(section.edges[pos],
                                 currentWeight,
                                 lastBrickId,
                                 pos,
                                 edgeSectionPos);
    assert(currentWeight >= 0.0f);
    currentWeight -= processEdge(section.edges[pos + 1],
                                 currentWeight,
                                 lastBrickId,
                                 pos + 1,
                                 edgeSectionPos);
    assert(currentWeight >= 0.0f);

    if((1 + section.edges[pos].brickId + section.edges[pos].lastBrickId * 2) % 3 == 0)
    {
        currentWeight -= nextEdgeSectionStep(section,
                                             pos * 2,
                                             currentWeight,
                                             section.edges[pos].brickId,
                                             edgeSectionPos);
        assert(currentWeight >= 0.0f);
    }

    if((1 + section.edges[pos + 1].brickId + section.edges[pos + 1].lastBrickId * 2) % 3 == 0)
    {
        currentWeight -= nextEdgeSectionStep(section,
                                             (pos + 1) * 2,
                                             currentWeight,
                                             section.edges[pos].brickId,
                                             edgeSectionPos);
        assert(currentWeight >= 0.0f);
    }

    // return the used weight
    return weight - currentWeight;
}

/**
 * @brief processEdgeSection
 * @param section
 * @param weight
 */
inline void
processEdgeSection(EdgeSection &section,
                   const float weight,
                   const uint32_t edgeSectionPos)
{
    nextEdgeSectionStep(section,
                        1,
                        weight,
                        UNINIT_STATE_32,
                        edgeSectionPos);
}

}

#endif // EDGE_PROCESSING_H
