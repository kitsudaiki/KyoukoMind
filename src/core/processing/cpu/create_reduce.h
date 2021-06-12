#ifndef CORE_CREATE_RESUCE_H
#define CORE_CREATE_RESUCE_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

#include <libKitsunemimiAiCommon/metadata.h>

/**
 * @brief createNewSynapse
 * @param section
 * @param synapse
 * @param bricks
 * @param randomValues
 * @param sourceNode
 * @param segmentMeta
 * @param synapseMetaData
 * @param remainingWeight
 */
inline void
createNewSynapse(SynapseSection* section,
                 Synapse* synapse,
                 Brick* bricks,
                 uint32_t* randomValues,
                 Node* sourceNode,
                 CoreSegmentMeta* segmentMeta,
                 Kitsunemimi::Ai::CoreMetaData* synapseMetaData,
                 const float remainingWeight)
{
    float randomMulti = 0.0f;
    float random = 0.0f;
    float doLearn = 0.0f;
    uint32_t targetNodeIdInBrick = 0;
    uint32_t nodeOffset = 0;
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
    synapse->border = static_cast<uint8_t>(synapse->weight) + 0.0001f;

    // update weight with multiplicator
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    randomMulti = static_cast<float>(randomValues[section->randomPos]) / RAND_MAX;
    synapse->weight *= randomMulti * static_cast<float>(synapseMetaData->multiplicatorRange);

    // update weight with multiplicator
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    signRand = randomValues[section->randomPos] % 1000;
    synapse->weight *= static_cast<float>(1 - (1000.0f * synapseMetaData->signNeg > signRand) * 2);

    // set target node id
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    nodeBrick = &bricks[sourceNode->nodeBrickId];
    const uint32_t targetBrickId = nodeBrick->possibleTargetNodeBrickIds[section->brickBufferPos];
    nodeOffset = nodeBrick->possibleTargetNodeBrickIds[section->brickBufferPos]
                 * segmentMeta->numberOfNodesPerBrick;
    Brick* targetBrick = &bricks[targetBrickId];
    if(targetBrick->isOutputBrick) {
        // TODO
        targetNodeIdInBrick = randomValues[section->randomPos] % 10;
    } else {
        targetNodeIdInBrick = randomValues[section->randomPos] % segmentMeta->numberOfNodesPerBrick;
    }
    synapse->targetNodeId = static_cast<uint16_t>(targetNodeIdInBrick + nodeOffset);
}

/**
 * @brief reduceCoreSynapses
 * @param segmentMeta
 * @param synapseBuffers
 * @param synapseSections
 * @param nodes
 * @param synapseMetaData
 * @param threadId
 * @param numberOfThreads
 */
inline void
reduceCoreSynapses(CoreSegmentMeta* segmentMeta,
                   SynapseSection* synapseSections,
                   Node* nodes,
                   Kitsunemimi::Ai::CoreMetaData*,
                   const uint32_t threadId,
                   const uint32_t numberOfThreads)
{
    const uint64_t numberOfSynapses = segmentMeta->numberOfSynapseSections;

    for(uint32_t i = threadId; i < numberOfSynapses; i = i + numberOfThreads)
    {
        bool upToData = 1;
        SynapseSection* section = &synapseSections[i];

        // iterate over all synapses in synapse-section
        uint32_t currentPos = section->hardening;
        for(uint32_t lastPos = section->hardening; lastPos < SYNAPSES_PER_SYNAPSESECTION; lastPos++)
        {
            Synapse* synapse = &section->synapses[lastPos];
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                continue;
            }

            upToData = 0;

            // update dynamic-weight-value of the synapse
            if(nodes[synapse->targetNodeId].active == 0) {
                synapse->activeCounter -= 2;
            } else {
                synapse->activeCounter -= 1;
            }

            // check for deletion of the single synapse
            if(synapse->activeCounter < 0)
            {
                synapse->weight = 0.0f;
                synapse->targetNodeId = UNINIT_STATE_16;
            }
            else
            {
                const Synapse currentSyn = section->synapses[currentPos];
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
