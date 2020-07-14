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
         const uint16_t pos,
         const float weight)
{
    if(weight < 0.1f) {
        return;
    }

    // try to create new synapse-section
    if((edge.synapseSectionId == UNINIT_STATE_32
            && static_cast<uint32_t>(rand()) % 3 == 0)
            || pos > 101)
    {
        SynapseSection newSection;
        const uint64_t newPos = KyoukoRoot::m_segment->synapses.addNewItem(newSection);
        assert(newPos != UNINIT_STATE_64);
        edge.synapseSectionId = static_cast<uint32_t>(newPos);
    }

    if(pos <= 101)
    {
        // update weight
        float edgeWeight = 0.0f;
        if(edge.synapseSectionId != UNINIT_STATE_32)
        {
            const float randRatio = static_cast<float>(static_cast<uint32_t>(rand()) % 1024) / 1024.0f;
            edgeWeight = weight * randRatio;
            edge.synapseWeight += weight * (1.0f - randRatio);
        }
        else
        {
            edgeWeight = weight;
        }

        const float randRatio = static_cast<float>(static_cast<uint32_t>(rand()) % 1024) / 1024.0f;
        edge.edgeWeight1 += edgeWeight * randRatio;
        edge.edgeWeight2 += edgeWeight * (1.0f - randRatio);

        //std::cout<<"+++ learn update: "<<(weight * randValue)<<std::endl;
    }
    else
    {
        edge.synapseWeight += weight;
    }

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
    if(pos > 203
            || weight < 0.1f)
    {
        return;
    }

    // get initial values
    Edge* edge = &section.edges[pos];

    // init new edge, if necessary
    if(edge->currentBrickId == UNINIT_STATE_32)
    {
        Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[lastBrickId];
        edge->currentBrickId = brick->getRandomNeighbor(UNINIT_STATE_32);
    }

    // calculate and process ratio
    const float totalWeight = edge->edgeWeight1
                              + edge->edgeWeight2
                              + edge->synapseWeight
                              + 0.0000001f;
    float ratio = weight / totalWeight;
    assert(ratio >= 0.0f);
    if(ratio > 1.0f) {
        lernEdge(*edge, pos, weight - totalWeight);
    }

    // limit ratio to 1.0
    ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;

    // process connection to synapse
    //processSynapseConnection(*edge, ratio, pos, edgeSectionPos);

    // update remaining weight based on the ratio
    nextEdgeSectionStep(section,
                        pos * 2,
                        edge->edgeWeight1 * ratio,
                        section.edges[pos].currentBrickId,
                        edgeSectionPos);

    nextEdgeSectionStep(section,
                        (pos * 2) + 1,
                        edge->edgeWeight2 * ratio,
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

    float test = 0.0f;
    for(uint32_t i = 0; i < 204; i++)
    {
        test += section.edges[i].synapseWeight;
    }
    std::cout<<"test: "<<test<<"   weight: "<<weight<<std::endl;
    assert(weight >= test);
}

}

#endif // EDGE_PROCESSING_H
