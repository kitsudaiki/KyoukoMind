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

    // try to create new synapse-section
    if(edge.synapseSectionId == UNINIT_STATE_32
            && static_cast<uint32_t>(rand()) % 3 == 0)
    {
        SynapseSection newSection;
        const uint64_t newPos = KyoukoRoot::m_segment->synapses.addNewItem(newSection);
        assert(newPos != UNINIT_STATE_64);
        edge.synapseSectionId = static_cast<uint32_t>(newPos);
    }

    // update weight
    if(edge.synapseSectionId != UNINIT_STATE_32)
    {
        const float randRatio = static_cast<float>(static_cast<uint32_t>(rand()) % 1024) / 1024.0f;
        edge.edgeWeight += weight * randRatio;
        edge.synapseWeight += weight * (1.0f - randRatio);
    }
    else
    {
        edge.edgeWeight += weight;
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
inline float
processSynapseConnection(Edge &edge,
                         const float ratio,
                         const uint16_t pos,
                         const uint32_t edgeSectionPos)
{
    //std::cout<<"--- edge.weight: "<<edge.weight<<std::endl;
    const float weight = edge.synapseWeight * ratio;
    if(edge.synapseSectionId != UNINIT_STATE_32
            && weight >= 0.01f)
    {
        //return usedWeight;
        SynapseTransfer newTransfer;
        newTransfer.weight = weight;
        newTransfer.brickId = edge.currentBrickId;
        newTransfer.positionInEdge = static_cast<uint8_t>(pos);
        newTransfer.sourceEdgeId = edgeSectionPos;
        //std::cout<<"SynapseTransfer-weight: "<<usedWeight<<std::endl;

        const uint64_t x = KyoukoRoot::m_segment->synapseTransfers.addNewItem(newTransfer);
        assert(x != UNINIT_STATE_64);
        return weight;
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
inline void
nextEdgeSectionStep(EdgeSection &section,
                    const uint16_t pos,
                    const float weight,
                    const uint32_t lastBrickId,
                    const uint32_t edgeSectionPos)
{
    // end-condition
    if(pos > 254
            || weight < 0.1f)
    {
        return;
    }

    // get initial values
    float currentWeight = weight;
    Edge* edge = &section.edges[pos];

    // init new edge, if necessary
    if(edge->currentBrickId == UNINIT_STATE_32)
    {
        Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[lastBrickId];
        edge->currentBrickId = brick->getRandomNeighbor(UNINIT_STATE_32);
    }

    // calculate and process ratio
    const float totalWeight = edge->edgeWeight + 0.0000001f;
    float ratio = weight / totalWeight;
    assert(ratio >= 0.0f);
    if(ratio > 1.0f) {
        lernEdge(*edge, weight - totalWeight);
    }

    // limit ratio to 1.0
    ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;

    // process connection to synapse
    currentWeight -= processSynapseConnection(*edge, ratio, pos, edgeSectionPos);

    // update remaining weight based on the ratio
    currentWeight = ratio * currentWeight;

    nextEdgeSectionStep(section,
                        pos * 2,
                        currentWeight,
                        section.edges[pos].currentBrickId,
                        edgeSectionPos);

    nextEdgeSectionStep(section,
                        (pos * 2) + 1,
                        currentWeight,
                        section.edges[pos].currentBrickId,
                        edgeSectionPos);

    // return the used weight
    return;
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
