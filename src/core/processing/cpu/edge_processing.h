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
#include <core/objects/global_values.h>

#include <core/connection_handler/client_connection_handler.h>

/**
 * @brief createSynapse
 * @param section
 * @param edge
 * @param weight
 */
inline bool
createSynapseSection(EdgeSection &section,
                     Edge &edge,
                     Brick* brick,
                     Brick* sourceBrick)
{
    // input-brick are never allowed to directly connect to the output-brick
    if(sourceBrick->isInputBrick
            && brick->isOutputBrick)
    {
        return false;
    }

    // init new synapse-section and register it inside the edge
    if(brick->isInputBrick == false
            && edge.synapseSectionId == UNINIT_STATE_32  // no synapse already exist
            && brick->nodeBrickId != UNINIT_STATE_32  // current brick must be a node-brick
            && section.numberOfUsedSynapseSections < 32)
    {
        brick->synapseCreateActivity++;
        section.numberOfUsedSynapseSections++;

        SynapseSection newSection;
        const uint64_t newPos = KyoukoRoot::m_segment->synapses.addNewItem(newSection);
        if(newPos == UNINIT_STATE_64) {
            return false;
        }

        edge.synapseSectionId = static_cast<uint32_t>(newPos);
        // very left bit says if this connection is new
        edge.synapseSectionId = edge.synapseSectionId | 0x80000000;
        edge.synapseWeight = 0.0f;

        return true;
    }

    return false;
}

/**
 * @brief initBrick
 * @param section
 * @param edge
 * @param lastLocation
 */
inline uint16_t
createNewEdge(EdgeSection &section,
              Edge &oldEdge,
              Brick* sourceBrick)
{
    // prepare
    const uint32_t oldBrickId = getBrickId(oldEdge.brickLocation);
    Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[oldBrickId];

    // get limiter for new run
    uint32_t* randValues = getBuffer<uint32_t>(KyoukoRoot::m_segment->randomIntValues);
    section.randomPos = (section.randomPos + 1) % 1024;
    const uint16_t maxRuns = randValues[section.randomPos] % 10;

    // iterate to a random new brick
    uint32_t possibleNextLoc = oldEdge.brickLocation;
    uint16_t counter = 0;
    while(counter < maxRuns)
    {
        possibleNextLoc = brick->getRandomNeighbor(possibleNextLoc);
        if(possibleNextLoc == UNINIT_STATE_32) {
            break;
        }
        brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(possibleNextLoc)];
        counter++;
    }

    // check if found brick is valid
    if(brick->nodeBrickId == UNINIT_STATE_32
            || possibleNextLoc == UNINIT_STATE_32)
    {
        return UNINIT_STATE_16;
    }

    // create new edge
    Edge newEdge;
    if(createSynapseSection(section, newEdge, brick, sourceBrick))
    {
        newEdge.brickLocation = possibleNextLoc;
        brick->edgeCreateActivity++;
        return section.append(newEdge);
    }

    return UNINIT_STATE_16;
}

/**
 * @brief processSynapseConnection
 * @param edge
 * @param weight
 * @param positionInSection
 * @param edgeSectionPos
 */
inline void
processSynapseConnection(Edge &edge,
                         const float weight,
                         const uint16_t positionInSection,
                         const uint32_t edgeSectionPos)
{
    // prepare
    Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[getBrickId(edge.brickLocation)];
    assert(brick->nodeBrickId != UNINIT_STATE_32);

    //return usedWeight;
    SynapseTransfer newTransfer;
    newTransfer.synapseSectionId = edge.synapseSectionId & 0x7FFFFFFF;
    newTransfer.isNew = edge.synapseSectionId >> 31;
    newTransfer.weight = weight;
    newTransfer.brickId = brick->brickId;
    newTransfer.nodeBrickId = brick->nodeBrickId;
    newTransfer.positionInEdge = positionInSection;
    newTransfer.sourceEdgeId = edgeSectionPos;

    KyoukoRoot::m_segment->synapseTransfers.addNewItem(newTransfer);
    brick->synapseActivity++;

    // set the left bit to 0 to mark this connections as not new anymore
    edge.synapseSectionId = edge.synapseSectionId & 0x7FFFFFFF;
}

/**
 * @brief processEdgeGroup
 * @param section
 * @param weight
 * @param brickId
 * @param edgeSectionPos
 * @param sourceBrick
 */
inline void
processEdgeGroup(EdgeSection &section,
                 float weight,
                 const uint32_t brickId,
                 const uint32_t edgeSectionPos,
                 Brick* sourceBrick)
{
    // prepare
    Edge* currentEdge = &section.edges[0];
    uint16_t currentPos = 0;
    currentEdge->brickLocation = brickId;

    // init learning
    float toLearn = weight - section.getTotalWeight();
    toLearn = (toLearn < 0.0f) * 0.0f + (toLearn >= 0.0f) * toLearn;

    // iterate over the linked list of edges
    while(currentEdge->next != UNINIT_STATE_16
          && weight > 0.0f)
    {
        currentPos = currentEdge->next;
        currentEdge = &section.edges[currentEdge->next];
        assert(currentEdge->synapseSectionId != UNINIT_STATE_32);

        // share learning-weight
        const float diff = toLearn * (1.0f - currentEdge->hardening);
        currentEdge->synapseWeight += diff;
        toLearn -= diff;

        // trigger synapse
        if(weight > currentEdge->synapseWeight)
        {
            processSynapseConnection(*currentEdge,
                                     currentEdge->synapseWeight,
                                     currentPos,
                                     edgeSectionPos);
            weight -= currentEdge->synapseWeight;
        }
        else
        {
            processSynapseConnection(*currentEdge,
                                     weight,
                                     currentPos,
                                     edgeSectionPos);
            weight = 0.0f;
            return;
        }
    }

    // if not everything of the new weight was shared, then create a new edge for the remaining
    if(toLearn > 2.0f)
    {
        // try to create new edge
        const uint16_t pos = createNewEdge(section, section.edges[0], sourceBrick);
        if(pos == UNINIT_STATE_16) {
            return;
        }

        // process the new created edge
        Edge* newEdge = &section.edges[pos];
        const uint32_t brickId = getBrickId(newEdge->brickLocation);
        Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[brickId];
        assert(brick->nodeBrickId != UNINIT_STATE_32);

        if(brick->isOutputBrick) {
            std::cout<<"create output from source: "<<sourceBrick->brickId<<" : "<<toLearn<<std::endl;
        }

        newEdge->synapseWeight = toLearn;
        processSynapseConnection(*newEdge,
                                 toLearn,
                                 pos,
                                 edgeSectionPos);
    }
}

/**
 * @brief processEdgeSection
 * @return
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
    GlobalValues* globalValues = getBuffer<GlobalValues>(segment->globalValues);

    // insert input-values from brick
    const std::vector<float> inputValues = bricks[1].getInputValues();
    for(uint32_t i = 0; i < inputValues.size(); i++)
    {
        const float multi = static_cast<float>(pow(1.05, static_cast<double>(i % 10)));
        axonTransfers[i].weight = inputValues.at(i) * multi;
        axonTransfers[i].brickId = 1;
    }

    // process axon-messages
    for(uint32_t i = 0; i < segment->axonTransfers.itemCapacity; i++)
    {
        const AxonTransfer* currentTransfer = &axonTransfers[i];
        if(currentTransfer->weight == 0.0f) {
            continue;
        }

        EdgeSection* currentSection = &edgeSections[i];
        Brick* sourceBrick = &bricks[currentTransfer->brickId];
        sourceBrick->nodeActivity++;

        // harden section
        if(globalValues->lerningValue != 0.0f) {
            currentSection->harden(globalValues->lerningValue);
        }

        //if(brick->isOutputBrick == false)
        if(sourceBrick->brickId != 60)
        {
            processEdgeGroup(*currentSection,
                             currentTransfer->weight,
                             currentSection->targetBrickId,
                             i,
                             sourceBrick);
        }
        else
        {
            const uint32_t offset = sourceBrick->nodePos;
            assert(sourceBrick->nodePos >= 41000);
            const uint32_t max = sourceBrick->getNumberOfOutputValues();
            if(max > 0) {
                sourceBrick->setOutputValue((i - offset) % max, currentTransfer->weight);
            }
        }

        // increase counter for monitoring-output
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
        // skip containers without a target
        if(container->targetId == UNINIT_STATE_32) {
            continue;
        }

        // iterade over all update-containers coming from the gpu
        EdgeSection* section = &edgeSections[container->targetId];
        Edge* edge = &section->edges[container->positionInEdge];
        assert(getBrickId(edge->brickLocation) != UNINIT_STATE_24);

        if(container->deleteEdge > 0)
        {
            KyoukoRoot::m_segment->synapses.deleteDynamicItem(edge->synapseSectionId);
            const uint32_t brickId = getBrickId(edge->brickLocation);
            Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[brickId];
            brick->synapseDeleteActivity++;
            brick->edgeDeleteActivity++;

            brick->edgeCreateActivity--;
            section->numberOfUsedSynapseSections--;
            assert(section->numberOfUsedSynapseSections >= 0.0f);
            //section->numberOfUsedSynapseSections = (numSyn < 0) * 0 + (numSyn >= 0) * numSyn;

            section->remove(container->positionInEdge);
        }
        else
        {
            edge->synapseWeight = container->newWeight;
        }

        // increase counter for monitoring-output
        count++;
    }

    return count;
}

#endif // EDGE_PROCESSING_H

