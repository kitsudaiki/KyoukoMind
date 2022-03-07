/**
 * @file        processing.h
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

#ifndef KYOUKOMIND_DYNAMIC_PROCESSING_H
#define KYOUKOMIND_DYNAMIC_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/segments/brick.h>

#include "objects.h"
#include "dynamic_segment.h"

/**
 * @brief add new basic synapse-section to segment
 *
 * @param segment refernce to segment
 *
 * @return position in buffer, where the section was added
 */
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
createNewSynapse(SynapseSection &section,
                 Synapse* synapse,
                 Brick* bricks,
                 const DynamicNode &sourceNode,
                 const DynamicSegmentSettings &segmentSettings,
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

    const float maxWeight = segmentSettings.maxSynapseWeight;

    // set new weight
    section.randomPos = (section.randomPos + 1) % NUMBER_OF_RAND_VALUES;
    random = static_cast<float>(randomValues[section.randomPos]) / randMax;
    doLearn = maxWeight * random;
    synapse->weight = static_cast<float>(remainingWeight < doLearn) * remainingWeight
                      + static_cast<float>(remainingWeight >= doLearn) * doLearn;

    // set activation-border
    synapse->border = (synapse->weight * 255.0f) + 1;

    // update weight with multiplicator
    section.randomPos = (section.randomPos + 1) % NUMBER_OF_RAND_VALUES;
    randomMulti = static_cast<float>(randomValues[section.randomPos]) / randMax;
    synapse->weight *= randomMulti * static_cast<float>(segmentSettings.multiplicatorRange) + 1.0f;

    // update weight with multiplicator
    section.randomPos = (section.randomPos + 1) % NUMBER_OF_RAND_VALUES;
    signRand = randomValues[section.randomPos] % 1000;
    synapse->weight *= static_cast<float>(1 - (1000.0f * segmentSettings.signNeg > signRand) * 2);

    // set target node id
    section.randomPos = (section.randomPos + 1) % NUMBER_OF_RAND_VALUES;
    nodeBrick = &bricks[sourceNode.brickId];
    const uint32_t targetBrickId = nodeBrick->possibleTargetNodeBrickIds[section.brickBufferPos];

    Brick* targetBrick = &bricks[targetBrickId];
    targetNodeIdInBrick = randomValues[section.randomPos] % targetBrick->numberOfNodes;

    synapse->targetNodeId = static_cast<uint16_t>(targetNodeIdInBrick + targetBrick->nodePos);
    synapse->activeCounter = 1;
    section.updated = 1;
}

/**
 * @brief process synapse-section
 *
 * @param section current processed synapse-section
 * @param segment refernece to the processed segment
 * @param sourceNode source-node, who triggered the section
 * @param weightIn wight-value, which comes into the section
 * @param outH multiplicator
 */
inline void
synapseProcessing(SynapseSection &section,
                  DynamicSegment &segment,
                  const DynamicNode &sourceNode,
                  const float weightIn,
                  const float outH)
{
    uint32_t pos = 0;
    float netH = weightIn;
    bool createSyn = false;
    Synapse* synapse = nullptr;
    DynamicNode* targetNode = nullptr;
    section.updated = 0;
    uint8_t active = 0;

    // iterate over all synapses in the section
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && netH > 0.0f)
    {
        synapse = &section.synapses[pos];
        createSyn = synapse->targetNodeId == UNINIT_STATE_16
                    && segment.dynamicSegmentSettings->doLearn > 0;
        // create new synapse if necesarry and learning is active
        if(createSyn)
        {
            createNewSynapse(section,
                             synapse,
                             segment.bricks,
                             sourceNode,
                             *segment.dynamicSegmentSettings,
                             netH);
        }

        // break loop, if learning is disabled to the loop has reached an inactive synapse
        if(synapse->targetNodeId == UNINIT_STATE_16) {
            break;
        }
        else if(synapse->targetNodeId == 0)
        {
            pos++;
            netH -= static_cast<float>(synapse->border) * BORDER_STEP;
            continue;
        }

        // update target-node
        targetNode = &segment.nodes[synapse->targetNodeId];
        targetNode->input += outH * synapse->weight;

        // update active-counter
        active = (synapse->weight > 0 && targetNode->input > 0.0f);
        active = active || (synapse->weight < 0 && targetNode->input < 0.0f);
        synapse->activeCounter += active * static_cast<uint8_t>(synapse->activeCounter < 126);

        // update loop-counter
        netH -= static_cast<float>(synapse->border) * BORDER_STEP;
        pos++;
    }

    if(netH > 0.1f)
    {
        if(section.next == UNINIT_STATE_32)
        {
            const uint64_t newPos = createNewSection(segment);
            if(newPos == ITEM_BUFFER_UNDEFINE_POS) {
                return;
            }

            section.next = newPos;
        }

        synapseProcessing(segment.synapseSections[section.next],
                          segment,
                          sourceNode,
                          netH,
                          outH);
    }
}

/**
 * @brief reset single node
 *
 * @param node pointer to node to reset
 */
inline void
initNode(DynamicNode* node)
{
    const bool initNode = node->isInit == false && node->input > 0.1f;
    node->isInit = node->isInit || initNode;
    node->border = static_cast<float>(initNode) * node->input * 0.5f
                   + static_cast<float>(initNode == false) * node->border;
}

/**
 * @brief reset nodes of a input brick
 *
 * @param brick pointer to the brick
 * @param segment segment where the brick belongs to
 */
inline void
prepareNodesOfInputBrick(const Brick &brick,
                         DynamicSegment &segment)
{
    float outH = 0.0f;
    DynamicNode* node = nullptr;
    uint64_t newPos = 0;

    for(uint32_t nodeId = brick.nodePos;
        nodeId < brick.numberOfNodes + brick.nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];
        node->input = segment.inputTransfers[node->targetBorderId];
        node->potential = segment.dynamicSegmentSettings->potentialOverflow * node->input;
        initNode(node);
        node->input = 0.0f;

        if(node->potential > 0.0f)
        {
            if(node->targetSectionId == UNINIT_STATE_32)
            {
                newPos = createNewSection(segment);
                if(newPos == ITEM_BUFFER_UNDEFINE_POS) {
                    continue;
                }

                node->targetSectionId = newPos;
            }

            outH = 1.0f / (1.0f + exp(-1.0f * node->potential));
            synapseProcessing(segment.synapseSections[node->targetSectionId],
                              segment,
                              *node,
                              node->potential,
                              outH);
        }
    }
}

/**
 * @brief reset nodes of a output brick
 *
 * @param brick pointer to the brick
 * @param segment segment where the brick belongs to
 */
inline void
prepareNodesOfOutputBrick(const Brick &brick,
                          const DynamicSegment &segment)
{
    DynamicNode* node = nullptr;
    bool active = false;

    for(uint32_t nodeId = brick.nodePos;
        nodeId < brick.numberOfNodes + brick.nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];
        node->potential = segment.dynamicSegmentSettings->potentialOverflow * node->input;
        node->potential = 1.0f / (1.0f + exp(-1.0f * node->potential));

        initNode(node);

        active = node->potential > node->border;
        if(active) {
            segment.outputTransfers[node->targetBorderId] = node->potential;
        }

        node->input = 0.0f;
    }
}

/**
 * @brief reset nodes of a normal brick
 *
 * @param brick pointer to the brick
 * @param segment segment where the brick belongs to
 */
inline void
prepareNodesOfNormalBrick(const Brick &brick,
                          DynamicSegment &segment)
{
    bool active = false;
    float outH = 0.0f;
    DynamicNode* node = nullptr;
    uint64_t newPos = 0;


    for(uint32_t nodeId = brick.nodePos;
        nodeId < brick.numberOfNodes + brick.nodePos;
        nodeId++)
    {
        node = &segment.nodes[nodeId];
        node->potential = segment.dynamicSegmentSettings->potentialOverflow * node->input;
        initNode(node);
        node->input = 0.0f;

        active = node->potential > node->border;
        if(active)
        {
            if(node->targetSectionId == UNINIT_STATE_32)
            {
                newPos = createNewSection(segment);
                if(newPos == ITEM_BUFFER_UNDEFINE_POS) {
                    continue;
                }

                node->targetSectionId = newPos;
            }

            outH = 1.0f / (1.0f + exp(-1.0f * node->potential));
            synapseProcessing(segment.synapseSections[node->targetSectionId],
                              segment,
                              *node,
                              node->potential,
                              outH);
        }

        node->active = active;
    }
}

/**
 * @brief process all nodes within a specific brick and also all synapse-sections,
 *        which are connected to an active node
 *
 * @param segment segment to process
 */
void
prcessDynamicSegment(DynamicSegment &segment)
{
    const uint32_t numberOfBricks = segment.segmentHeader->bricks.count;
    for(uint32_t pos = 0; pos < numberOfBricks; pos++)
    {
        const uint32_t brickId = segment.brickOrder[pos];
        Brick* brick = &segment.bricks[brickId];
        if(brick->isInputBrick) {
            prepareNodesOfInputBrick(*brick, segment);
        } else if(brick->isOutputBrick) {
            prepareNodesOfOutputBrick(*brick, segment);
        } else {
            prepareNodesOfNormalBrick(*brick, segment);
        }
    }
}

#endif // KYOUKOMIND_DYNAMIC_PROCESSING_H
