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
 * @brief createSynapse
 * @param section
 * @param edge
 * @param weight
 */
inline void
createSynapse(EdgeSection &section,
              Edge &edge,
              const float weight,
              Brick* brick)
{
    uint32_t* randValues = getBuffer<uint32_t>(KyoukoRoot::m_segment->randomIntValues);
    section.randomPos = (section.randomPos + 1) % 1024;

    if(edge.synapseSectionId == UNINIT_STATE_32  // no synapse already exist
            && brick->nodeBrickId != UNINIT_STATE_32  // current brick must be a node-brick
            && randValues[section.randomPos] % 5 == 0  // random value to not create every time
            && weight >= 5.0f  // minimal weight for a new synapse-section
            && section.numberOfUsedSynapseSections < 32)
    {
        assert(brick->nodeBrickId != UNINIT_STATE_32);
        brick->synapseCreateActivity++;
        section.numberOfUsedSynapseSections++;

        SynapseSection newSection;
        const uint64_t newPos = KyoukoRoot::m_segment->synapses.addNewItem(newSection);
        assert(newPos != UNINIT_STATE_64);
        edge.synapseSectionId = static_cast<uint32_t>(newPos);
        // very left bit says if this connection is new
        edge.synapseSectionId = edge.synapseSectionId | 0x80000000;
    }

    edge.synapseWeight = (edge.synapseSectionId != UNINIT_STATE_32) * edge.synapseWeight;
}

/**
 * @brief lernEdge
 * @param edge
 * @param weight
 */
inline void
reweightEdge(EdgeSection &section,
             Edge &edge,
             const uint16_t positionInSection,
             float weight)
{
    uint32_t* randValues = getBuffer<uint32_t>(KyoukoRoot::m_segment->randomIntValues);

    // get random values
    section.randomPos = (section.randomPos + 1) % 1024;
    const float randRatio1 = static_cast<float>(randValues[section.randomPos] % 1024) / 1024.0f;
    section.randomPos = (section.randomPos + 1) % 1024;
    const float randRatio2 = static_cast<float>(randValues[section.randomPos] % 1024) / 1024.0f;

    // update synapse-weight
    const float synapseWeight = weight * randRatio1;
    weight -= synapseWeight;
    edge.synapseWeight += synapseWeight;

    // update weight in next edges
    section.edges[positionInSection * 2].edgeWeight += weight * randRatio2;
    section.edges[(positionInSection * 2) + 1].edgeWeight += weight * (1.0f - randRatio2);

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
                         const uint16_t positionInEdge,
                         const uint32_t edgeSectionPos,
                         Brick* brick)
{
    if(edge.synapseSectionId != UNINIT_STATE_32)
    {
        //return usedWeight;
        SynapseTransfer newTransfer;
        newTransfer.synapseSectionId = edge.synapseSectionId & 0x7FFFFFFF;
        newTransfer.isNew = edge.synapseSectionId >> 31;
        newTransfer.weight = edge.synapseWeight * ratio;
        newTransfer.brickId = brick->brickId;
        newTransfer.nodeBrickId = brick->nodeBrickId;
        assert(brick->nodeBrickId != UNINIT_STATE_32);
        newTransfer.positionInEdge = static_cast<uint8_t>(positionInEdge);
        newTransfer.sourceEdgeId = edgeSectionPos;

        // set the left bit to 0 to mark this connections as not new anymore
        edge.synapseSectionId = edge.synapseSectionId & 0x7FFFFFFF;

        KyoukoRoot::m_segment->synapseTransfers.addNewItem(newTransfer);
        brick->synapseActivity++;

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
                    const uint16_t positionInSection,
                    const float weight,
                    const uint32_t lastLocation,
                    const uint32_t edgeSectionPos)
{
    // end-condition
    if(weight < 0.1f) {
        return;
    }

    // prepare pointer
    Edge* edge = &section.edges[positionInSection];

    // init new edge, if necessary
    if(getBrickId(*edge) == UNINIT_STATE_24)
    {
        Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(lastLocation)];
        edge->brickLocation = brick->getRandomNeighbor(lastLocation);
        if(getBrickId(edge->brickLocation) == UNINIT_STATE_24) {
            return;
        }
        brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(edge->brickLocation)];
        brick->edgeCreateActivity++;
    }

    // handle output
    // if(edge->brickLocation >> 24 == 25) {
    //     brick->setOutputValue(edge->brickLocation & 0x00FFFFFF, weight);
    // }

    float ratio = 0.0f;
    const uint32_t idFromEdge = getBrickId(*edge);
    Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[idFromEdge];

    if(positionInSection < 127)
    {
        // prepare pointer
        Edge* child1 = &section.edges[(positionInSection * 2)];
        Edge* child2 = &section.edges[(positionInSection * 2) + 1];

        // calculate and process ratio
        const float totalWeight = edge->synapseWeight  + child1->edgeWeight + child2->edgeWeight;
        ratio = weight / (totalWeight + 0.0000001f);
        if(ratio > 1.0f)
        {
            reweightEdge(section, *edge, positionInSection, weight - totalWeight);
            createSynapse(section, *edge, edge->synapseWeight, brick);
        }

        // limit ratio to 1.0
        ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;

        // update remaining weight based on the ratio
        nextEdgeSectionStep(section,
                            positionInSection * 2,
                            child1->edgeWeight * ratio,
                            edge->brickLocation,
                            edgeSectionPos);

        nextEdgeSectionStep(section,
                            (positionInSection * 2) + 1,
                            child2->edgeWeight * ratio,
                            edge->brickLocation,
                            edgeSectionPos);
    }
    else
    {
        // calculate and process ratio
        ratio = weight / (edge->synapseWeight + 0.0000001f);
        ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;

        edge->synapseWeight = edge->edgeWeight;
        createSynapse(section, *edge, edge->synapseWeight, brick);
    }

    // process connection to synapse
    processSynapseConnection(*edge,
                             ratio,
                             positionInSection,
                             edgeSectionPos,
                             brick);
}

/**
 * @brief resetEdge
 * @param edge
 */
inline void
resetEdge(Edge &edge)
{
    if(getBrickId(edge) != UNINIT_STATE_24)
    {
        Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(edge)];
        brick->edgeDeleteActivity++;
    }

    edge.edgeWeight = 0.0f;
    edge.brickLocation = UNINIT_STATE_32;
    edge.synapseWeight = 0.0f;
}

/**
 * @brief cleanupEdgeSection
 * @param section
 */
inline void
cleanupEdgeSection(EdgeSection &section)
{
    for(uint32_t i = 253; i >= 127; i--)
    {
        Edge* edge = &section.edges[i];
        if(edge->synapseSectionId == UNINIT_STATE_32) {
            resetEdge(*edge);
        } else {
            edge->edgeWeight = edge->synapseWeight;
        }
    }

    for(uint32_t i = 126; i >= 1; i--)
    {
        Edge* edge = &section.edges[i];
        Edge* next1 = &section.edges[i * 2];
        Edge* next2 = &section.edges[(i * 2) + 1];

        if(edge->synapseSectionId == UNINIT_STATE_32
                && next1->edgeWeight == 0.0f
                && next2->edgeWeight == 0.0f)
        {
            resetEdge(*edge);
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

    // prepare pointer
    Segment* segment = KyoukoRoot::m_segment;
    EdgeSection* edgeSections = getBuffer<EdgeSection>(segment->edges);
    AxonTransfer* axonTransfers = getBuffer<AxonTransfer>(segment->axonTransfers);
    Brick* bricks = getBuffer<Brick>(segment->bricks);

    // insert input-values from brick
    const std::vector<float> inputValues = bricks[1].getInputValues();
    for(uint32_t i = 0; i < inputValues.size(); i++) {
        axonTransfers[i].weight = inputValues.at(i) * static_cast<float>(pow(1.05, static_cast<double>(i % 10)));
    }

    // process axon-messages
    for(uint32_t i = 0; i < segment->axonTransfers.itemCapacity; i++)
    {
        const AxonTransfer* currentTransfer = &axonTransfers[i];
        if(currentTransfer->weight == 0.0f) {
            continue;
        }

        EdgeSection* currentSection = &edgeSections[i];
        Brick* brick = &bricks[currentTransfer->brickId];
        brick->nodeActivity++;

        //if(brick->isOutputBrick == false)
        if(currentTransfer->brickId != 60)
        {
            cleanupEdgeSection(*currentSection);
            nextEdgeSectionStep(*currentSection,
                                1,
                                currentTransfer->weight,
                                currentSection->targetBrickId,
                                i);
        }
        else
        {
            const uint32_t offset = brick->nodePos;
            const uint32_t max = brick->getNumberOfOutputValues();
            if(max > 0) {
                brick->setOutputValue((i - offset) % max, currentTransfer->weight);
            }
        }

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

    for(const UpdateTransfer* container = start;
        container < end;
        container++)
    {
        if(container->targetId == UNINIT_STATE_32) {
            continue;
        }

        float newWeight = container->newWeight;
        newWeight = (newWeight < 0.0f) * 0.0f + (newWeight >= 0.0f) * newWeight;

        EdgeSection* secstion = &edgeSections[container->targetId];
        Edge* edge = &secstion->edges[container->positionInEdge];
        if(container->deleteEdge > 0)
        {
            KyoukoRoot::m_segment->synapses.deleteDynamicItem(edge->synapseSectionId);
            Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(*edge)];
            brick->synapseDeleteActivity++;
            const int16_t numSyn = secstion->numberOfUsedSynapseSections;
            secstion->numberOfUsedSynapseSections = (numSyn < 0) * 0 + (numSyn >= 0) * numSyn;

            secstion->numberOfUsedSynapseSections--;
            edge->synapseSectionId = UNINIT_STATE_32;
            edge->synapseWeight = 0;
        }
        else
        {
            edge->synapseWeight = newWeight;
        }

        count++;
    }

    return count;
}

#endif // EDGE_PROCESSING_H
