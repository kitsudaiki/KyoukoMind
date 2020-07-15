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
    if(weight < 5.0f) {
        return;
    }

    // try to create new synapse-section
    if((edge.synapseSectionId == UNINIT_STATE_32
            && static_cast<uint32_t>(rand()) % 3 == 0)
            || pos >= 127)
    {
        SynapseSection newSection;
        const uint64_t newPos = KyoukoRoot::m_segment->synapses.addNewItem(newSection);
        assert(newPos != UNINIT_STATE_64);
        edge.synapseSectionId = static_cast<uint32_t>(newPos);
    }

    if(pos < 127)
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
        section.edges[pos * 2].edgeWeight += edgeWeight * randRatio;
        section.edges[(pos * 2) + 1].edgeWeight += edgeWeight * (1.0f - randRatio);

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
inline void
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
        newTransfer.synapseSectionId = edge.synapseSectionId;
        newTransfer.weight = weight;
        newTransfer.brickId = edge.currentBrickId;
        newTransfer.positionInEdge = static_cast<uint8_t>(pos);
        newTransfer.sourceEdgeId = edgeSectionPos;
        std::cout<<"SynapseTransfer-positionInEdge: "<<(int)pos<<std::endl;

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
    if(pos > 254
            || weight < 5.0f)
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
    float totalWeight = edge->synapseWeight + 0.0000001f;
    if(pos < 127)
    {
        totalWeight += section.edges[pos * 2].edgeWeight;
        totalWeight += section.edges[(pos * 2) + 1].edgeWeight;
    }

    float ratio = weight / totalWeight;
    assert(ratio >= 0.0f);
    if(ratio > 1.0f) {
        lernEdge(section, *edge, pos, weight - totalWeight);
    }

    // limit ratio to 1.0
    ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;

    // process connection to synapse
    processSynapseConnection(*edge, ratio, pos, edgeSectionPos);

    if(pos < 127)
    {
        // update remaining weight based on the ratio
        nextEdgeSectionStep(section,
                            pos * 2,
                            section.edges[pos * 2].edgeWeight * ratio,
                            section.edges[pos].currentBrickId,
                            edgeSectionPos);

        nextEdgeSectionStep(section,
                            (pos * 2) + 1,
                            section.edges[(pos * 2) + 1].edgeWeight * ratio,
                            section.edges[pos].currentBrickId,
                            edgeSectionPos);
    }

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


    /*float test = 0.0f;
    for(uint32_t i = 0; i < 255; i++)
    {
        test += section.edges[i].synapseWeight;
    }
    std::cout<<"test: "<<test<<"   weight: "<<weight<<std::endl;
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
    return;
}

}

#endif // EDGE_PROCESSING_H
