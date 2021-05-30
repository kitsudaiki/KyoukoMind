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
 * @param sectionPos
 * @param weight
 * @param hardening
 */
inline void
synapseProcessing(SynapseSection* section,
                  Node* nodes,
                  Brick* bricks,
                  float* nodeBuffers,
                  SynapseBuffer* synapseBuffers,
                  CoreSegmentMeta* segmentMeta,
                  uint32_t* randomValues,
                  Kitsunemimi::Ai::CoreMetaData* synapseMetaData,
                  Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
                  const uint32_t nodeId,
                  const float weightIn,
                  const uint32_t layer)
{
    uint32_t pos = 0;
    uint32_t counter = 0;
    float weight = weightIn;
    bool processed = false;
    uint32_t signRand = 0;
    const float maxWeight = synapseMetaData->maxSynapseWeight;
    Node* node = &nodes[nodeId];

    uint32_t targetNodeIdInBrick = 0;
    uint32_t nodeOffset = 0;
    Brick* nodeBrick = nullptr;

    float randomMulti = 0.0f;
    float random = 0.0f;
    float doLearn = 0.0f;

    ulong nodeBufferPosition = 0;
    float ratio = 0.0f;
    float shareWeight = 0.0f;

    uint32_t bufferPos = 0;
    SynapseBuffer* synapseBuffer = nullptr;
    uint32_t nextLayer = 0;

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
            targetNodeIdInBrick = randomValues[section->randomPos] % segmentMeta->numberOfNodesPerBrick;
            nodeBrick = &bricks[node->nodeBrickId];
            nodeOffset = nodeBrick->possibleTargetNodeBrickIds[section->brickBufferPos]
                         * segmentMeta->numberOfNodesPerBrick;
            synapse->targetNodeId = static_cast<uint16_t>(targetNodeIdInBrick + nodeOffset);
        }

        pos++;

        // process synapse
        if(synapse->targetNodeId != UNINIT_STATE_16)
        {
            ratio = weight / static_cast<float>(synapse->border);
            shareWeight = static_cast<float>(ratio >= 1.0f) * synapse->weight
                          + static_cast<float>(ratio < 1.0f) * synapse->weight * ratio;

            // 0 because only one thread at the moment
            nodeBufferPosition = (0 * segmentMeta->numberOfNodes) + synapse->targetNodeId;
            nodeBuffers[nodeBufferPosition] += shareWeight;

            synapse->activeCounter += (synapse->activeCounter < 126) * 1;
            weight -= static_cast<float>(synapse->border);
            counter = pos;
            processed = true;
        }
    }

    // harden synapse-section
    const bool updatePos = networkMetaData->lerningValue > 0.0f
                           && counter > section->hardening;
    section->hardening = (updatePos == true) * counter
                         + (updatePos == false) * section->hardening;

    // go to next section
    if(weight > 1.0f
            && processed)
    {
        nextLayer = layer + 1;
        nextLayer = (nextLayer > 7) * 7  + (nextLayer <= 7) * nextLayer;
        bufferPos = (node->targetSectionId + nextLayer * 10000 + nextLayer) % segmentMeta->numberOfSynapseSections;
        synapseBuffer = &synapseBuffers[bufferPos];
        synapseBuffer->buffer[nextLayer].weigth = weight;
        synapseBuffer->buffer[nextLayer].nodeId = nodeId;
        synapseBuffer->process = 1;
    }
}

/**
 * @brief updating
 * @param sectionPos
 */
inline bool
updating(SynapseSection* section,
         Node* nodes,
         Kitsunemimi::Ai::CoreMetaData*)
{
    bool upToData = 1;

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
            synapse->activeCounter -= 0;
        } else {
            synapse->activeCounter -= 0;
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

    return upToData;
}

inline void
updateCoreSynapses(CoreSegmentMeta* segmentMeta,
                   SynapseBuffer* synapseBuffers,
                   SynapseSection* synapseSections,
                   Node* nodes,
                   Kitsunemimi::Ai::CoreMetaData* synapseMetaData,
                   const uint32_t threadId,
                   const uint32_t numberOfThreads)
{
    const uint64_t numberOfSynapses = segmentMeta->numberOfSynapseSections;

    for(uint32_t i = threadId; i < numberOfSynapses; i = i + numberOfThreads)
    {
        SynapseBuffer* synapseBuffer = &synapseBuffers[i];

        const bool updateSection = synapseBuffer->process == 0
                                   && synapseBuffer->upToDate == 0;
        if(updateSection)
        {
            synapseBuffer->upToDate = updating(&synapseSections[i],
                                               nodes,
                                               synapseMetaData);
        }
    }
}

/**
 * @brief synapse_processing
 */
inline void
synapse_processing(CoreSegmentMeta* segmentMeta,
                   SynapseBuffer* synapseBuffers,
                   SynapseSection* synapseSections,
                   Node* nodes,
                   Brick* bricks,
                   float* nodeBuffers,
                   uint32_t* randomValues,
                   Kitsunemimi::Ai::CoreMetaData* synapseMetaData,
                   Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
                   const uint32_t threadId,
                   const uint32_t numberOfThreads)
{
    const uint64_t numberOfSynapses = segmentMeta->numberOfSynapseSections;

    //----------------------------------------------------------------------------------------------
    for(uint32_t i = threadId; i < numberOfSynapses; i = i + numberOfThreads)
    {
        SynapseBuffer* synapseBuffer = &synapseBuffers[i];

        if(synapseBuffer->process == 0) {
            continue;
        }
        synapseBuffer->process = 0;

        for(uint8_t layer = 0; layer < 8; layer++)
        {
            SynapseBufferEntry* entry = &synapseBuffer->buffer[layer];
            synapseProcessing(&synapseSections[i],
                              nodes,
                              bricks,
                              nodeBuffers,
                              synapseBuffers,
                              segmentMeta,
                              randomValues,
                              synapseMetaData,
                              networkMetaData,
                              entry->nodeId,
                              entry->weigth,
                              layer);
            synapseBuffer->upToDate = 0;

            entry->weigth = 0.0f;
            entry->nodeId = UNINIT_STATE_32;
        }
    }
}

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
 * @brief node_processing
 */
inline void
node_processing(Node* nodes,
                float* nodeBuffers,
                SynapseBuffer* synapseBuffers,
                CoreSegmentMeta* segmentMeta,
                Kitsunemimi::Ai::CoreMetaData* synapseMetaData,
                OutputInput* outputInputs,
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

            synapseBuffers[i].buffer[0].weigth = node->potential;
            synapseBuffers[i].buffer[0].nodeId = i;
            synapseBuffers[i].process |= node->potential > 5.0f;

            // post-steps
            node->refractionTime = node->refractionTime >> 1;
            node->potential /= synapseMetaData->nodeCooldown;
            node->currentState /= synapseMetaData->nodeCooldown;
        }
        else if(node->border == 0.0f)
        {
            synapseBuffers[i].buffer[0].weigth = node->potential;
            synapseBuffers[i].buffer[0].nodeId = i;
            synapseBuffers[i].process |= node->potential > 5.0f;
        }
        else
        {
            OutputInput* oIn = &outputInputs[i % segmentMeta->numberOfNodesPerBrick];
            const bool isNew = oIn->weight > node->currentState * 1.01f
                               || oIn->weight < node->currentState * 0.99f;
            oIn->isNew = isNew + (isNew == false) * oIn->isNew;
            oIn->weight = node->currentState;
            node->currentState = 0.0f;
        }
    }
}

#endif // SYNAPSE_PROCESSING_H
