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
    if(weight < 0.1f) {
        return;
    }

    // update weight
    const float randValue = static_cast<float>(static_cast<uint32_t>(rand()) % 1024) / 1024.0f;
    edge.weight += weight * randValue;
    //std::cout<<"+++ learn update: "<<(weight * randValue)<<std::endl;

    return;
}

/**
 * @brief initNewEdge
 * @param edge
 * @param lastBrickId
 */
inline void
initNewEdge(Edge &edge,
            const uint32_t lastBrickId)
{
    Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[lastBrickId];
    edge.brickId = brick->getRandomNeighbor(UNINIT_STATE_32);
    edge.lastBrickId = lastBrickId;

    // try to create new synapse-section
    if(edge.synapseSectionId == UNINIT_STATE_32
            && static_cast<uint32_t>(rand()) % 3 == 0)
    {
        SynapseSection newSection;
        const uint64_t newPos = KyoukoRoot::m_segment->synapses.addNewItem(newSection);
        assert(newPos != UNINIT_STATE_64);
        edge.synapseSectionId = static_cast<uint32_t>(newPos);
    }
}

/**
 * @brief processSynapseConnection
 * @param edge
 * @param weight
 * @param pos
 * @param edgeSectionPos
 * @return
 */
inline float
processSynapseConnection(Edge &edge,
                         const float weight,
                         const uint16_t pos,
                         const uint32_t edgeSectionPos)
{
    float ratio = weight / edge.weight;
    assert(ratio >= 0.0f);
    if(ratio > 1.0f) {
        lernEdge(edge, weight - edge.weight);
    }
    ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;

    const float usedWeight = edge.weight * ratio;
    //std::cout<<"--- edge.weight: "<<edge.weight<<std::endl;
    if(usedWeight >= 0.01f)
    {
        //return usedWeight;
        SynapseTransfer newTransfer;
        newTransfer.weight = usedWeight;
        newTransfer.brickId = edge.brickId;
        newTransfer.positionInEdge = static_cast<uint8_t>(pos);
        newTransfer.sourceEdgeId = edgeSectionPos;
        //std::cout<<"SynapseTransfer-weight: "<<usedWeight<<std::endl;

        const uint64_t x = KyoukoRoot::m_segment->synapseTransfers.addNewItem(newTransfer);
        assert(x != UNINIT_STATE_64);
        return usedWeight;
    }

    return 0.0f;
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
    if(weight < 0.1f) {
        return 0.0f;
    }

    //std::cout<<"##### process pos: "<<pos<<"   weight: "<<weight<<std::endl;
    // set brick-ids, if not already done
    if(edge.brickId == UNINIT_STATE_32) {
        initNewEdge(edge, lastBrickId);
    }

    if(edge.synapseSectionId != UNINIT_STATE_32) {
        return processSynapseConnection(edge, weight, pos, edgeSectionPos);
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
            || weight < 0.1f)
    {
        return 0.0f;
    }

    float currentWeight = weight;

    currentWeight -= processEdge(section.edges[pos],
                                 currentWeight * 0.5f,
                                 lastBrickId,
                                 pos,
                                 edgeSectionPos);

    currentWeight -= nextEdgeSectionStep(section,
                                         pos * 2,
                                         currentWeight * 0.66f,
                                         section.edges[pos].brickId,
                                         edgeSectionPos);

    currentWeight -= nextEdgeSectionStep(section,
                                         (pos * 2) + 1,
                                         currentWeight * 0.33f,
                                         section.edges[pos].brickId,
                                         edgeSectionPos);

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
                   const uint32_t edgeSectionPos,
                   const uint32_t brickId)
{
    nextEdgeSectionStep(section,
                        1,
                        weight,
                        brickId,
                        edgeSectionPos);
}

}

#endif // EDGE_PROCESSING_H
