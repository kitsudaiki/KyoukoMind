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
        newTransfer.synapseSectionId = edge.synapseSectionId;
        newTransfer.weight = weight;
        newTransfer.brickId = edge.currentBrickId;
        newTransfer.positionInEdge = static_cast<uint8_t>(pos);
        newTransfer.sourceEdgeId = edgeSectionPos;
        //std::cout<<"SynapseTransfer-positionInEdge: "<<(int)pos<<std::endl;

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
                    const uint32_t lastBrickId,
                    const uint32_t edgeSectionPos)
{
    // end-condition
    if(weight < 0.1f) {
        return;
    }

    // get initial values
    Edge* edge = &section.edges[pos];

    // init new edge, if necessary
    if(edge->currentBrickId == UNINIT_STATE_32)
    {
        Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[lastBrickId];
        edge->currentBrickId = brick->getRandomNeighbor(edge->currentBrickId);
    }

    float ratio = 0.0f;

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
                            edge->currentBrickId,
                            edgeSectionPos);

        nextEdgeSectionStep(section,
                            (pos * 2) + 1,
                            section.edges[(pos * 2) + 1].edgeWeight * ratio,
                            edge->currentBrickId,
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
        if(KyoukoRoot::m_segment->synapses.numberOfItems == 0) {
            std::cout<<"i: "<<i<<"   weight: "<<section.edges[i].synapseWeight<<std::endl;
        }
    }
    std::cout<<"################################## test: "<<test<<"   weight: "<<weight<<std::endl;
    assert(weight >= test);*/
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
                  const float weightDiff,
                  const uint8_t deleteEdge)
{
    assert(posInSection < 255);

    uint8_t pos = posInSection;
    float diff = weightDiff;

    Edge* edge = &section.edges[pos];

    // process synapse-connection
    if(edge->synapseSectionId != UNINIT_STATE_32)
    {
        if(deleteEdge != 0)
        {
            assert(KyoukoRoot::m_segment->synapses.deleteDynamicItem(edge->synapseSectionId));
            diff = edge->synapseWeight;
            edge->synapseWeight = 0;
            edge->synapseSectionId = UNINIT_STATE_32;
        }
        else
        {
            edge->synapseWeight -= diff;
        }

        const float tempWeight = edge->synapseWeight;
        edge->synapseWeight = (tempWeight < 1.0f) * 0.0f + (tempWeight >= 0.0f) * tempWeight;
    }
    else
    {
        edge->synapseWeight = 0;
    }

    // shouldn't be necessary, but only to be sure
    if(pos >= 127
            && edge->synapseSectionId == UNINIT_STATE_32)
    {
        edge->edgeWeight = 0.0f;
        edge->currentBrickId = UNINIT_STATE_32;
        edge->synapseWeight = 0.0f;
    }

    // process edges
    while(pos > 2)
    {
        if(pos < 127)
        {
            edge->edgeWeight -= diff;

            if(edge->edgeWeight < 1.0f
                    && deleteEdge != 0)
            {
                diff += edge->edgeWeight;
                edge->edgeWeight = 0.0f;
                edge->currentBrickId = UNINIT_STATE_32;
            }

            const float tempWeight = edge->edgeWeight;
            edge->edgeWeight = (tempWeight < 1.0f) * 0.0f + (tempWeight >= 0.0f) * tempWeight;
        }
        else
        {
            if(edge->synapseSectionId == UNINIT_STATE_32)
            {
                edge->edgeWeight = 0.0f;
                edge->currentBrickId = UNINIT_STATE_32;
                edge->synapseWeight = 0.0f;
            }
        }

        pos = pos >> 1;
        edge = &section.edges[pos];
    }

    return;
}

}

#endif // EDGE_PROCESSING_H
