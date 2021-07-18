#ifndef CORE_CREATE_RESUCE_H
#define CORE_CREATE_RESUCE_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

/**
 * @brief createNewSynapse
 * @param section
 * @param synapse
 * @param bricks
 * @param randomValues
 * @param sourceNode
 * @param synapseMetaData
 * @param remainingWeight
 */
inline void
createNewSynapse(SynapseSection* section,
                 Synapse* synapse,
                 Brick* bricks,
                 uint32_t* randomValues,
                 Node* sourceNode,
                 SegmentSettings* synapseMetaData,
                 const float remainingWeight)
{
    float randomMulti = 0.0f;
    float random = 0.0f;
    float doLearn = 0.0f;
    uint32_t targetNodeIdInBrick = 0;
    Brick* nodeBrick = nullptr;
    uint32_t signRand = 0;

    const float maxWeight = synapseMetaData->maxSynapseWeight;

    // set new weight
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    random = static_cast<float>(randomValues[section->randomPos]) / RAND_MAX;
    doLearn = maxWeight * random;
    synapse->weight = static_cast<float>(remainingWeight < doLearn) * remainingWeight
                      + static_cast<float>(remainingWeight >= doLearn) * doLearn;

    // set activation-border
    synapse->border = (synapse->weight * 255.0f) + 1;

    // update weight with multiplicator
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    randomMulti = static_cast<float>(randomValues[section->randomPos]) / RAND_MAX;
    synapse->weight *= randomMulti * static_cast<float>(synapseMetaData->multiplicatorRange) + 1.0f;

    // update weight with multiplicator
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    signRand = randomValues[section->randomPos] % 1000;
    synapse->weight *= static_cast<float>(1 - (1000.0f * synapseMetaData->signNeg > signRand) * 2);

    // set target node id
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    nodeBrick = &bricks[sourceNode->nodeBrickId];
    const uint32_t targetBrickId = nodeBrick->possibleTargetNodeBrickIds[section->brickBufferPos];

    Brick* targetBrick = &bricks[targetBrickId];
    targetNodeIdInBrick = randomValues[section->randomPos] % targetBrick->numberOfNodes;

    synapse->targetNodeId = static_cast<uint16_t>(targetNodeIdInBrick + targetBrick->nodePos);
    synapse->activeCounter = 1;
    section->updated = 1;
}

/**
 * @brief hardenSynapses
 * @param nodes
 * @param synapseSections
 * @param segmentHeader
 */
inline void
hardenSynapses(Segment* segment)
{
    uint16_t pos = 0;
    Node* sourceNode = nullptr;
    SynapseSection* section = nullptr;
    Synapse* synapse = nullptr;
    uint32_t counter = 0;
    float netH = 0.0f;
    bool updateHardening = false;

    for(uint32_t nodeId = 0;
        nodeId < segment->segmentHeader->nodes.count;
        nodeId++)
    {
        sourceNode = &segment->nodes[nodeId];
        section = &segment->synapseSections[nodeId];

        if(section->active == 0
                || section->updated == 0)
        {
            continue;
        }

        if(sourceNode->input > 0.0f) {
            sourceNode->init = 1;
        }

        counter = 0;
        pos = 0;
        netH = sourceNode->potential;

        // iterate over all synapses in the section and update the target-nodes
        while(pos < SYNAPSES_PER_SYNAPSESECTION
              && netH > 0.0f)
        {
            synapse = &section->synapses[pos];
            pos++;

            // process synapse
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                break;
            }

            netH -= static_cast<float>(synapse->border) * BORDER_STEP;
            counter = pos;
        }

        // harden synapse-section
        updateHardening = counter > section->hardening;
        section->hardening = (updateHardening == true) * counter
                             + (updateHardening == false) * section->hardening;
    }
}

/**
 * @brief reduceCoreSynapses
 * @param segmentHeader
 * @param synapseSections
 * @param nodes
 */
inline void
reduceCoreSynapses(Segment* segment)
{
    Synapse currentSyn;
    uint32_t currentPos = 0;
    SynapseSection* section = nullptr;
    Synapse* synapse = nullptr;
    bool upToData = false;

    for(uint32_t sectionId = 0;
        sectionId < segment->segmentHeader->synapseSections.count;
        sectionId++)
    {
        upToData = 1;
        section = &segment->synapseSections[sectionId];

        // iterate over all synapses in synapse-section
        currentPos = section->hardening;
        for(uint32_t lastPos = section->hardening; lastPos < SYNAPSES_PER_SYNAPSESECTION; lastPos++)
        {
            synapse = &section->synapses[lastPos];
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                continue;
            }

            upToData = 0;

            // update dynamic-weight-value of the synapse
            if(segment->nodes[synapse->targetNodeId].active == 0) {
                synapse->activeCounter = -2;
            } else {
                synapse->activeCounter = -2;
            }

            // check for deletion of the single synapse
            if(synapse->activeCounter < 0)
            {
                synapse->weight = 0.0f;
                synapse->targetNodeId = UNINIT_STATE_16;
                synapse->border = 0;
            }
            else
            {
                currentSyn = section->synapses[currentPos];
                section->synapses[currentPos] = section->synapses[lastPos];
                section->synapses[lastPos] = currentSyn;
                currentPos++;
            }
        }

        // delete if sections is empty
        if(section->hardening == 0
                && currentPos == 0)
        {
            section->active = 0;
            upToData = 1;
        }
    }
}

#endif // CORE_CREATE_RESUCE_H