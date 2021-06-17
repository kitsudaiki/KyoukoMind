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
    synapse->border = (synapse->weight + 0.0001f) * 255.0f;

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
}

/**
 * @brief backpropagateNodes
 * @param nodes
 * @param synapseSections
 * @param startPoint
 * @param s
 */
inline void
hardenSynapses(Node* nodes,
               SynapseSection* synapseSections,
               CoreSegmentMeta* segmentMeta)
{
    const float borderStep = 1.0f / 255.0f;

    for(uint32_t nodeId = 0; nodeId < segmentMeta->numberOfNodes; nodeId++)
    {
        Node* sourceNode = &nodes[nodeId];
        SynapseSection* section = &synapseSections[nodeId];

        if(section->active == 0) {
            continue;
        }

        uint32_t counter = 0;
        uint16_t pos = 0;
        float netH = sourceNode->potential;

        // iterate over all synapses in the section and update the target-nodes
        while(pos < SYNAPSES_PER_SYNAPSESECTION
              && netH > 0.0f)
        {
            Synapse* synapse = &section->synapses[pos];
            pos++;

            // process synapse
            if(synapse->targetNodeId != UNINIT_STATE_16)
            {
                netH -= static_cast<float>(synapse->border) * borderStep;
                counter = pos;
            }
        }

        // harden synapse-section
        const bool updateHardening = counter > section->hardening;
        section->hardening = (updateHardening == true) * counter
                             + (updateHardening == false) * section->hardening;
    }
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
                   Node* nodes)
{
    for(uint32_t i = 0; i < segmentMeta->numberOfSynapseSections; i++)
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
