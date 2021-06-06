/**
 * @file        synapse_processing.h
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

#ifndef SYNAPSE_PROCESSING_H
#define SYNAPSE_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

#include <libKitsunemimiAiCommon/metadata.h>

/**
 * @brief synapseProcessing
 * @param section
 * @param nodes
 * @param bricks
 * @param nodeBuffers
 * @param segmentMeta
 * @param randomValues
 * @param synapseMetaData
 * @param networkMetaData
 * @param nodeId
 * @param weightIn
 */
inline void
synapseProcessing(SynapseSection* section,
                  Node* nodes,
                  Brick* bricks,
                  float* nodeBuffers,
                  CoreSegmentMeta* segmentMeta,
                  uint32_t* randomValues,
                  Kitsunemimi::Ai::CoreMetaData* synapseMetaData,
                  Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
                  Node* node,
                  const float weightIn)
{

    if(node->potential <= 5.0f) {
        return;
    }

    uint32_t pos = 0;
    uint32_t counter = 0;
    float weight = weightIn;
    bool processed = false;
    uint32_t signRand = 0;
    const float maxWeight = synapseMetaData->maxSynapseWeight;

    uint32_t targetNodeIdInBrick = 0;
    uint32_t nodeOffset = 0;
    Brick* nodeBrick = nullptr;

    float randomMulti = 0.0f;
    float random = 0.0f;
    float doLearn = 0.0f;

    ulong nodeBufferPosition = 0;
    float ratio = 0.0f;

    // reinit section if necessary
    if(section->active == 0)
    {
        section->active = 1;
        section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
        section->brickBufferPos = randomValues[section->randomPos] % 1000;
    }

    // iterate over all synapses in the section and update the target-nodes
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && weight > 0.0f)
    {
        Synapse* synapse = &section->synapses[pos];

        // create new synapse
        const bool createSyn = synapse->targetNodeId == UNINIT_STATE_16
                               && pos >= section->hardening
                               && networkMetaData->doLearn > 0;
        if(createSyn)
        {
            // set new weight
            section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
            random = static_cast<float>(randomValues[section->randomPos]) / RAND_MAX;
            doLearn = maxWeight * random;
            synapse->weight = static_cast<float>(weight < doLearn) * weight
                              + static_cast<float>(weight >= doLearn) * doLearn;

            // set activation-border
            synapse->border = static_cast<uint8_t>(synapse->weight) + 1;

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
            nodeBrick = &bricks[node->nodeBrickId];
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

        pos++;

        // process synapse
        if(synapse->targetNodeId != UNINIT_STATE_16)
        {
            ratio = weight / static_cast<float>(synapse->border);
            ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;
            ratio *= -1.0f * pow(0.98f, pos) + 1.0f;

            const float delta = nodes[synapse->targetNodeId].delta;
            const float learnValue =  0.5f;
            synapse->weight -= learnValue * delta * ratio;

            // 0 because only one thread at the moment
            nodeBufferPosition = (0 * segmentMeta->numberOfNodes) + synapse->targetNodeId;
            nodeBuffers[nodeBufferPosition] += ratio * synapse->weight;

            synapse->activeCounter += (synapse->activeCounter < 126);
            weight -= static_cast<float>(synapse->border);
            counter = pos;
            processed = true;
        }
    }

    // harden synapse-section
    const bool updateHardening = networkMetaData->lerningValue > 0.0f
                                 && counter > section->hardening;
    section->hardening = (updateHardening == true) * counter
                         + (updateHardening == false) * section->hardening;
}

/**
 * @brief node_processing
 * @param nodes
 * @param nodeBuffers
 * @param synapseBuffers
 * @param segmentMeta
 * @param synapseMetaData
 * @param outputInputs
 * @param threadId
 * @param numberOfThreads
 */
inline void
node_processing(Node* nodes,
                float* nodeBuffers,
                CoreSegmentMeta* segmentMeta,
                SynapseSection* synapseSections,
                Brick* bricks,
                uint32_t* randomValues,
                Kitsunemimi::Ai::CoreMetaData* synapseMetaData,
                Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
                const uint32_t threadId,
                const uint32_t numberOfThreads)
{
    for(uint32_t i = threadId; i < segmentMeta->numberOfNodes; i = i + numberOfThreads)
    {
        // TODO: when port to gpu: change 2 to 256 again
        for(uint pos = 0; pos < 1; pos++)
        {
            const ulong nodeBufferPosition = (pos * (segmentMeta->numberOfNodes)) + i;
            nodes[i].currentState += nodeBuffers[nodeBufferPosition];
            nodeBuffers[nodeBufferPosition] = 0.0f;
        }

        Node* node = &nodes[i];
        node->delta = 0.0f;

        if(node->border > 0.0f)
        {
            // check if active
            const bool reset = node->border < node->currentState
                               && node->refractionTime == 0;
            if(reset)
            {
                node->potential = synapseMetaData->actionPotential +
                                  synapseMetaData->potentialOverflow * node->currentState;
                node->refractionTime = synapseMetaData->refractionTime;
            }

            synapseProcessing(&synapseSections[i],
                              nodes,
                              bricks,
                              nodeBuffers,
                              segmentMeta,
                              randomValues,
                              synapseMetaData,
                              networkMetaData,
                              node,
                              node->potential);

            // post-steps
            node->active = node->currentState > node->border;
            node->refractionTime = node->refractionTime >> 1;
            node->potential /= synapseMetaData->nodeCooldown;
            node->currentState /= synapseMetaData->nodeCooldown;
        }
        else if(node->border == 0.0f)
        {
            synapseProcessing(&synapseSections[i],
                              nodes,
                              bricks,
                              nodeBuffers,
                              segmentMeta,
                              randomValues,
                              synapseMetaData,
                              networkMetaData,
                              node,
                              node->potential);
        }
    }
}

/**
 * @brief updateCoreSynapses
 * @param segmentMeta
 * @param synapseSections
 * @param nodes
 * @param synapseMetaData
 * @param threadId
 * @param numberOfThreads
 */
inline void
updateCoreSynapses(CoreSegmentMeta* segmentMeta,
                   SynapseSection* synapseSections,
                   Node* nodes,
                   OutputNode* outputNodes,
                   Brick* bricks,
                   Kitsunemimi::Ai::CoreMetaData* synapseMetaData,
                   const uint32_t threadId,
                   const uint32_t numberOfThreads)
{
    for(uint64_t i = 0; i < segmentMeta->numberOfOutputs; i++)
    {
        OutputNode* out = &outputNodes[i];
        Node* targetNode = &nodes[out->targetNode];
        targetNode->delta = ((out->outputWeight - out->shouldValue) * (-0.020202 * (out->outputWeight - 255)));
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

/**
 * @brief processInputNodes
 * @param nodes
 * @param inputNodes
 * @param segmentMeta
 */
inline void
processInputNodes(Node* nodes,
                  InputNode* inputNodes,
                  CoreSegmentMeta* segmentMeta)
{
    for(uint64_t i = 0; i < segmentMeta->numberOfInputs; i++) {
        nodes[inputNodes[i].targetNode].potential = inputNodes[i].weight;
    }
}

/**
 * @brief processOutputNodes
 * @param nodes
 * @param outputNodes
 * @param segmentMeta
 */
inline void
processOutputNodes(Node* nodes,
                   OutputNode* outputNodes,
                   CoreSegmentMeta* segmentMeta)
{
    for(uint64_t i = 0; i < segmentMeta->numberOfOutputs; i++)
    {
        OutputNode* out = &outputNodes[i];
        Node* targetNode = &nodes[out->targetNode];
        float nodeWeight = targetNode->currentState;
        nodeWeight = (nodeWeight < 0.0f) * 0.0f + (nodeWeight >= 0.0f) * nodeWeight;

        out->outputWeight = (-1.0f * pow(0.98f, nodeWeight) + 1.0f) * 255.0f;
        targetNode->currentState = 0.0f;
        targetNode->delta = 0.0f;
    }
}

#endif // SYNAPSE_PROCESSING_H
