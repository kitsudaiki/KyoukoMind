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

#include <core/connection_handler/client_connection_handler.h>

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
    Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(edge.brickLocation)];
    brick->activity++;
    section.randomPos = (section.randomPos + 1) % 1024;

    if(edge.synapseSectionId == UNINIT_STATE_32
            && brick->nodeBrickId != UNINIT_STATE_32
            && randValues[section.randomPos] % 5 == 0
            && weight >= 5.0f)
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

        // get random values
        section.randomPos = (section.randomPos + 1) % 1024;
        const float randRatio1 = static_cast<float>(randValues[section.randomPos] % 1024) / 1024.0f;
        section.randomPos = (section.randomPos + 1) % 1024;
        const float randRatio2 = static_cast<float>(randValues[section.randomPos] % 1024) / 1024.0f;

        // update synapse-weight
        const uint8_t hasSynapse = edge.synapseSectionId != UNINIT_STATE_32;
        edgeWeight -= hasSynapse * (weight * randRatio1);
        edge.synapseWeight += hasSynapse * (weight * randRatio1);

        // update weight in next edges
        section.edges[pos * 2].edgeWeight += edgeWeight * randRatio2;
        section.edges[(pos * 2) + 1].edgeWeight += edgeWeight * (1.0f - randRatio2);
    }
    else
    {
        const uint8_t hasSynapse = edge.synapseSectionId != UNINIT_STATE_32;
        edge.synapseWeight = hasSynapse * edge.edgeWeight;
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
        newTransfer.weight = edge.synapseWeight * ratio;
        newTransfer.brickId = getBrickId(edge.brickLocation);
        newTransfer.positionInEdge = static_cast<uint8_t>(pos);
        newTransfer.sourceEdgeId = edgeSectionPos;

        // set the left bit to 0 to mark this connections as not new anymore
        edge.synapseSectionId = edge.synapseSectionId & 0x7FFFFFFF;

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
    // end-condition
    if(weight < 0.1f) {
        return;
    }

    // prepare pointer
    Edge* edge = &section.edges[pos];
    Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(lastLocation)];

    // init new edge, if necessary
    if(getBrickId(edge->brickLocation) == UNINIT_STATE_24)
    {
        edge->brickLocation = brick->getRandomNeighbor(lastLocation);
        brick->activity++;
    }

    // handle output
    // if(edge->brickLocation >> 24 == 25) {
    //     brick->setOutputValue(edge->brickLocation & 0x00FFFFFF, weight);
    // }

    float ratio = 0.0f;

    if(getBrickId(edge->brickLocation) != UNINIT_STATE_24)
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
                                edge->brickLocation,
                                edgeSectionPos);

            nextEdgeSectionStep(section,
                                (pos * 2) + 1,
                                section.edges[(pos * 2) + 1].edgeWeight * ratio,
                                edge->brickLocation,
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
 * @brief resetEdge
 * @param edge
 */
inline void
resetEdge(Edge &edge)
{
    if(edge.synapseSectionId != UNINIT_STATE_32) {
        KyoukoRoot::m_segment->synapses.deleteDynamicItem(edge.synapseSectionId);
    }

    edge.synapseSectionId = UNINIT_STATE_32;
    edge.edgeWeight = 0.0f;
    edge.brickLocation = UNINIT_STATE_32;
}

/**
 * @brief cleanupEdgeSection
 * @param section
 */
inline void
cleanupEdgeSection(EdgeSection &section)
{
    for(uint32_t i = 255; i >= 2; i = i - 2)
    {
        Edge* edge1 = &section.edges[i];  // i is unequal
        Edge* edge2 = &section.edges[i - 1];  // i is equal
        Edge* parent = &section.edges[(i - 1) / 2];

        if(edge1->edgeWeight == 0.0f) {
            resetEdge(*edge1);
        }

        if(edge2->edgeWeight == 0.0f) {
            resetEdge(*edge2);
        }

        parent->edgeWeight = edge1->edgeWeight + edge2->edgeWeight + parent->synapseWeight;
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

    // prepare pointer
    Segment* segment = KyoukoRoot::m_segment;
    EdgeSection* edgeSections = getBuffer<EdgeSection>(segment->edges);
    AxonTransfer* axonTransfers = getBuffer<AxonTransfer>(segment->axonTransfers);
    Brick* brick = getBuffer<Brick>(segment->bricks);

    // insert input-values from brick
    const std::vector<float> inputValues = brick[1].getInputValues();
    for(uint32_t i = 0; i < inputValues.size(); i++) {
        axonTransfers[i].weight = inputValues.at(i);
    }

    // process axon-messages
    for(uint32_t i = 0; i < segment->axonTransfers.itemCapacity; i++)
    {
        cleanupEdgeSection(edgeSections[i]);
        nextEdgeSectionStep(edgeSections[i],
                            1,
                            axonTransfers[i].weight,
                            edgeSections[i].targetBrickId,
                            i);

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

    for(UpdateTransfer* container = start;
        container < end;
        container++)
    {
        if(container->targetId == UNINIT_STATE_32) {
            continue;
        }

        Edge* edge = &edgeSections[container->targetId].edges[container->positionInEdge];
        const uint8_t notDel = container->deleteEdge == 0;
        edge->synapseWeight = notDel * container->newWeight;

        count++;
    }

    return count;
}

#endif // EDGE_PROCESSING_H
