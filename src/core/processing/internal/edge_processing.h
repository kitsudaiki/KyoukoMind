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
    // try to create new synapse-section
    if((edge.synapseSectionId == UNINIT_STATE_32
            && static_cast<uint32_t>(rand()) % 3 == 0))
    {
        SynapseSection newSection;
        const uint64_t newPos = KyoukoRoot::m_segment->synapses.addNewItem(newSection);
        assert(newPos != UNINIT_STATE_64);
        edge.synapseSectionId = static_cast<uint32_t>(newPos);
        edge.synapseSectionId = edge.synapseSectionId | 0x80000000;
    }

    // update weight in current edge
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

    if(pos < 127)
    {
        // update weight in next edges
        const float randRatio = static_cast<float>(static_cast<uint32_t>(rand()) % 1024) / 1024.0f;
        section.edges[pos * 2].edgeWeight += edgeWeight * randRatio;
        section.edges[(pos * 2) + 1].edgeWeight += edgeWeight * (1.0f - randRatio);
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
    //std::cout<<"--- edge.weight: "<<edge.weight<<std::endl;
    const float weight = edge.synapseWeight * ratio;
    if(edge.synapseSectionId != UNINIT_STATE_32)
    {
        //return usedWeight;
        SynapseTransfer newTransfer;
        newTransfer.synapseSectionId = edge.synapseSectionId & 0x7FFFFFFF;
        newTransfer.isNew = edge.synapseSectionId >> 31;
        newTransfer.weight = weight;
        newTransfer.brickId = getBrickId(edge.location);
        newTransfer.positionInEdge = static_cast<uint8_t>(pos);
        newTransfer.sourceEdgeId = edgeSectionPos;

        edge.synapseSectionId = edge.synapseSectionId & 0x7FFFFFFF;
        //std::cout<<"SynapseTransfer-positionInEdge: "<<(int)pos<<"    brick-id: "<<newTransfer.brickId<<std::endl;

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
        if(pos < 127)
        {
            // calculate and process ratio
            const float totalWeight = edge->synapseWeight + 0.0000001f
                                      + section.edges[pos * 2].edgeWeight
                                      + section.edges[(pos * 2) + 1].edgeWeight;

            ratio = weight / totalWeight;
            if(ratio > 1.0f) {
                lernEdge(section, *edge, pos, weight - totalWeight);
            }
            if(ratio <= 0.0f) {
                std::cout<<"ratio: "<<ratio<<std::endl;
                assert(false);
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
            if(ratio <= 0.0f) {
                assert(false);
            }

            // limit ratio to 1.0
            ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;
        }

        // process connection to synapse
        processSynapseConnection(*edge, ratio, pos, edgeSectionPos);
    }
    else {
        assert(edge->synapseWeight == 0.0f);
    }

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


    /*float test = 0.0f;
    for(uint32_t i = 1; i < 255; i++)
    {
        test += section.edges[i].synapseWeight;
        std::cout<<"i: "<<i<<"   weight: "<<section.edges[i].edgeWeight<<std::endl;

    }
    std::cout<<"################################## test: "<<test<<"   weight: "<<weight<<std::endl;*/
}

/**
 * @brief updateEdgeSection
 * @param section
 * @param posInSection
 * @param weightDiff
 * @param deleteEdge
 */
inline void
updateEdgeSection(EdgeSection &section,
                  const uint8_t posInSection,
                  const float newWeight,
                  const uint8_t deleteEdge)
{
    assert(posInSection < 255);

    uint8_t pos = posInSection;

    Edge* edge = &section.edges[pos];

    // TODO: find out, why this id can be in uninit-state here, because this should exist,
    //       but somehow it works. I don't get it and that's not good, even if it works ...
    if(edge->synapseSectionId == UNINIT_STATE_32) {
        return;
    }

    // process synapse-connection
    if(deleteEdge != 0)
    {
        assert(KyoukoRoot::m_segment->synapses.deleteDynamicItem(edge->synapseSectionId));
        edge->synapseWeight = 0;
        edge->synapseSectionId = UNINIT_STATE_32;
    }
    else
    {
        edge->synapseWeight = newWeight;
        assert(edge->synapseWeight >= 0.0f);
    }

    // process edges
    while(pos > 2)
    {
        if(pos < 127)
        {
            const Edge edge1 = section.edges[pos * 2];
            const Edge edge2 = section.edges[(pos * 2) + 1];

            if(getBrickId(edge1.location) == UNINIT_STATE_24
                    && getBrickId(edge2.location) == UNINIT_STATE_24
                    && edge->synapseSectionId == UNINIT_STATE_32)
            {
                edge->edgeWeight = 0.0f;
                edge->location = UNINIT_STATE_32;
                edge->synapseWeight = 0.0f;
            }
            else
            {
                edge->edgeWeight = edge1.edgeWeight + edge2.edgeWeight + edge->synapseWeight;
            }
        }
        else
        {
            if(edge->synapseSectionId == UNINIT_STATE_32)
            {
                edge->edgeWeight = 0.0f;
                edge->location = UNINIT_STATE_32;
                edge->synapseWeight = 0.0f;
            }
            else
            {
                edge->edgeWeight = edge->synapseWeight;
            }
        }

        pos = pos >> 1;
        edge = &section.edges[pos];
    }

    return;
}

}

#endif // EDGE_PROCESSING_H
