/**
 * @file        create_reduce.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#ifndef KYOUKOMIND_CREATE_RESUCE_H
#define KYOUKOMIND_CREATE_RESUCE_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/orchestration/segments/dynamic_segment.h>
#include <core/objects/synapses.h>
#include <core/orchestration/network_cluster.h>

inline uint64_t
createNewSection(DynamicSegment &segment)
{
    SynapseSection newSection;
    newSection.active = Kitsunemimi::ItemBuffer::ACTIVE_SECTION;
    newSection.randomPos = rand() % NUMBER_OF_RAND_VALUES;
    newSection.brickBufferPos = KyoukoRoot::m_randomValues[newSection.randomPos] % 1000;

    return segment.segmentData.addNewItem(newSection);
}

/**
 * @brief initialize a new specific synapse
 *
 * @param section current processed synapse-section
 * @param synapse new synapse, which has to be initialized
 * @param bricks array of all bricks
 * @param sourceNode source-node, who triggered the section
 * @param segmentSettings settings of the section
 * @param remainingWeight weight of which to cut of a part for the new synapse
 */
inline void
createNewSynapse(SynapseSection* section,
                 Synapse* synapse,
                 Brick* bricks,
                 Node* sourceNode,
                 SegmentSettings* segmentSettings,
                 const float remainingWeight)
{
    float randomMulti = 0.0f;
    float random = 0.0f;
    float doLearn = 0.0f;
    uint32_t targetNodeIdInBrick = 0;
    Brick* nodeBrick = nullptr;
    uint32_t signRand = 0;
    const uint32_t* randomValues = KyoukoRoot::m_randomValues;
    const float randMax = static_cast<float>(RAND_MAX);

    const float maxWeight = segmentSettings->maxSynapseWeight;

    // set new weight
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    random = static_cast<float>(randomValues[section->randomPos]) / randMax;
    doLearn = maxWeight * random;
    synapse->weight = static_cast<float>(remainingWeight < doLearn) * remainingWeight
                      + static_cast<float>(remainingWeight >= doLearn) * doLearn;

    // set activation-border
    synapse->border = (synapse->weight * 255.0f) + 1;

    // update weight with multiplicator
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    randomMulti = static_cast<float>(randomValues[section->randomPos]) / randMax;
    synapse->weight *= randomMulti * static_cast<float>(segmentSettings->multiplicatorRange) + 1.0f;

    // update weight with multiplicator
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    signRand = randomValues[section->randomPos] % 1000;
    synapse->weight *= static_cast<float>(1 - (1000.0f * segmentSettings->signNeg > signRand) * 2);

    // set target node id
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    nodeBrick = &bricks[sourceNode->brickId];
    const uint32_t targetBrickId = nodeBrick->possibleTargetNodeBrickIds[section->brickBufferPos];

    Brick* targetBrick = &bricks[targetBrickId];
    targetNodeIdInBrick = randomValues[section->randomPos] % targetBrick->numberOfNodes;

    synapse->targetNodeId = static_cast<uint16_t>(targetNodeIdInBrick + targetBrick->nodePos);
    synapse->activeCounter = 1;
    section->updated = 1;
}

/**
 * @brief hardenSynapses
 * @param segment
 */
inline void
hardenSynapses(DynamicSegment* segment,
               SynapseSection* section,
               float netH)
{
    uint16_t pos = 0;
    Synapse* synapse = nullptr;
    bool updateHardening = false;

    // iterate over all synapses in the section
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && netH > 0.0f)
    {
        // break look, if no more synapses to process
        synapse = &section->synapses[pos];
        if(synapse->targetNodeId == UNINIT_STATE_16) {
            break;
        }

        // update loop-counter
        netH -= static_cast<float>(synapse->border) * BORDER_STEP;
        pos++;
    }

    // harden synapse-section
    updateHardening = pos > section->hardening;
    section->hardening = (updateHardening == true) * pos
                         + (updateHardening == false) * section->hardening;

    if(section->next != UNINIT_STATE_32) {
        hardenSynapses(segment, &segment->synapseSections[section->next], netH);
    }
}


/**
 * @brief harden all synapses within a specific section
 *
 * @param segment current segemnt to process
 */
inline void
hardenSegment(DynamicSegment* segment)
{
    Node* node = nullptr;

    for(uint32_t nodeId = 0;
        nodeId < segment->segmentHeader->nodes.count;
        nodeId++)
    {
        node = &segment->nodes[nodeId];
        /*if(sourceNode->input > 0.0f) {
            sourceNode->isInit = 1;
        }*/

        if(node->targetSectionId != UNINIT_STATE_32)
        {
            hardenSynapses(segment,
                           &segment->synapseSections[node->targetSectionId],
                           node->potential);
        }
    }
}

/**
 * @brief reduceSynapses
 * @param segment
 * @param section
 * @return
 */
inline bool
reduceSynapses(DynamicSegment* segment,
               SynapseSection* section)
{
    Synapse* synapse = nullptr;
    Synapse currentSyn;
    uint32_t currentPos = 0;

    // iterate over all synapses in synapse-section
    currentPos = section->hardening;
    for(uint32_t lastPos = section->hardening;
        lastPos < SYNAPSES_PER_SYNAPSESECTION;
        lastPos++)
    {
        // skip not connected synapses
        synapse = &section->synapses[lastPos];
        if(synapse->targetNodeId == UNINIT_STATE_16) {
            continue;
        }

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

    if(section->next != UNINIT_STATE_32)
    {
        // delete if sections is empty
        const bool shouldDelete = reduceSynapses(segment, &segment->synapseSections[section->next]);
        if(shouldDelete)
        {
            segment->segmentData.deleteItem(section->next);
            section->next = UNINIT_STATE_32;
        }
    }

    const bool shouldDelete = section->hardening == 0
                              && currentPos == 0
                              && section->next == UNINIT_STATE_32;
    return shouldDelete;
}

/**
 * @brief reduce all synapses within the segment and delete them, if the reach a deletion-border
 *
 * @param segment current segemnt to process
 */
inline void
reduceNodes(DynamicSegment* segment)
{
    SynapseSection* section = nullptr;
    Node* sourceNode = nullptr;
    uint32_t sectionId = 0;
    bool shouldDelete = false;

    for(uint32_t nodeId = 0;
        nodeId < segment->segmentHeader->nodes.count;
        nodeId++)
    {
        sourceNode = &segment->nodes[nodeId];
        if(sourceNode->targetSectionId == UNINIT_STATE_32) {
            continue;
        }

        // set start-values
        sectionId = sourceNode->targetSectionId;
        section = &segment->synapseSections[sectionId];

        // delete if sections is empty
        shouldDelete = reduceSynapses(segment, section);
        if(shouldDelete)
        {
            segment->segmentData.deleteItem(sectionId);
            sourceNode->targetSectionId = UNINIT_STATE_32;
        }
    }
}

#endif // KYOUKOMIND_CREATE_RESUCE_H
