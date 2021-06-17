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

#include <core/processing/cpu/create_reduce.h>

/**
 * @brief synapseProcessing
 * @param section
 * @param bricks
 * @param nodeBuffers
 * @param segmentMeta
 * @param randomValues
 * @param synapseMetaData
 * @param networkMetaData
 * @param sourceNode
 * @param weightIn
 */
inline void
synapseProcessing(SynapseSection* section,
                  Brick* bricks,
                  Node* nodes,
                  CoreSegmentMeta* segmentMeta,
                  uint32_t* randomValues,
                  Kitsunemimi::Ai::CoreMetaData* synapseMetaData,
                  Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
                  Node* sourceNode,
                  const float weightIn)
{
    const float borderStep = 1.0f / 255.0f;

    // reinit section if necessary
    if(section->active == 0)
    {
        section->active = 1;
        section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
        section->brickBufferPos = randomValues[section->randomPos] % 1000;
    }

    uint32_t pos = 0;
    float netH = weightIn;
    const float outH = 1.0f / (1.0f + exp(-1.0f * netH));

    // iterate over all synapses in the section and update the target-nodes
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && netH > 0.0f)
    {
        Synapse* synapse = &section->synapses[pos];
        const bool createSyn = synapse->targetNodeId == UNINIT_STATE_16
                               && pos >= section->hardening
                               && networkMetaData->doLearn > 0;
        if(createSyn)
        {
            createNewSynapse(section,
                             synapse,
                             bricks,
                             randomValues,
                             sourceNode,
                             segmentMeta,
                             synapseMetaData,
                             netH);
        }

        pos++;

        // process synapse
        if(synapse->targetNodeId != UNINIT_STATE_16)
        {
            netH -= static_cast<float>(synapse->border) * borderStep;
            nodes[synapse->targetNodeId].input += outH * synapse->weight;

            synapse->activeCounter += (synapse->activeCounter < 126);
        }
    }
}

/**
 * @brief processSynapseBuffer
 * @param nodes
 * @param segmentMeta
 * @param synapseSections
 * @param synapseBuffers
 * @param bricks
 * @param randomValues
 * @param synapseMetaData
 * @param networkMetaData
 */
inline void
processSynapseBuffer(Node* nodes,
                     CoreSegmentMeta* segmentMeta,
                     SynapseSection* synapseSections,
                     SynapseBuffer* synapseBuffers,
                     Brick* bricks,
                     uint32_t* randomValues,
                     Kitsunemimi::Ai::CoreMetaData* synapseMetaData,
                     Kitsunemimi::Ai::NetworkMetaData* networkMetaData)
{
    for(uint32_t bufferId = 0; bufferId < segmentMeta->numberOfSynapseSections; bufferId++)
    {
        SynapseBuffer* synapseBuffer = &synapseBuffers[bufferId];

        if(synapseBuffer->targetId != UNINIT_STATE_32)
        {
            synapseProcessing(&synapseSections[synapseBuffer->targetId],
                              bricks,
                              nodes,
                              segmentMeta,
                              randomValues,
                              synapseMetaData,
                              networkMetaData,
                              &nodes[synapseBuffer->nodeId],
                              synapseBuffer->weigth);

            synapseBuffer->weigth = 0.0f;
            synapseBuffer->nodeId = UNINIT_STATE_32;
            synapseBuffer->targetId = UNINIT_STATE_32;
        }
    }
}

/**
 * @brief node_processing
 * @param brick
 * @param nodes
 * @param synapseBuffers
 * @param synapseMetaData
 */
inline void
nodeProcessingMultiThread(Brick* brick,
                          Node* nodes,
                          SynapseBuffer* synapseBuffers,
                          Kitsunemimi::Ai::CoreMetaData* synapseMetaData)
{
    const uint32_t upperPos = brick->numberOfNodes + brick->nodePos;
    for(uint32_t nodeId = brick->nodePos; nodeId < upperPos; nodeId++)
    {
        Node* node = &nodes[nodeId];
        node->potential = synapseMetaData->potentialOverflow * node->input;
        node->input = 0.0f;
        node->delta = 0.0f;
    }

    for(uint32_t nodeId = brick->nodePos; nodeId < upperPos; nodeId++)
    {
        Node* node = &nodes[nodeId];
        if(node->border >= 0.0f
                && node->potential > 0.01f)
        {
            synapseBuffers[nodeId].weigth = node->potential;
            synapseBuffers[nodeId].nodeId = nodeId;
            synapseBuffers[nodeId].targetId = nodeId;
        }

        node->active = node->potential > node->border;
    }
}

/**
 * @brief nodeProcessingSingleThread
 * @param brick
 * @param nodes
 * @param segmentMeta
 * @param synapseSections
 * @param bricks
 * @param randomValues
 * @param synapseMetaData
 * @param networkMetaData
 */
inline void
nodeProcessingSingleThread(Brick* brick,
                           Node* nodes,
                           CoreSegmentMeta* segmentMeta,
                           SynapseSection* synapseSections,
                           Brick* bricks,
                           uint32_t* randomValues,
                           Kitsunemimi::Ai::CoreMetaData* synapseMetaData,
                           Kitsunemimi::Ai::NetworkMetaData* networkMetaData)
{
    const uint32_t upperPos = brick->numberOfNodes + brick->nodePos;
    for(uint32_t nodeId = brick->nodePos; nodeId < upperPos; nodeId++)
    {
        Node* node = &nodes[nodeId];
        node->potential = synapseMetaData->potentialOverflow * node->input;
        node->input = 0.0f;
        node->delta = 0.0f;
    }

    for(uint32_t nodeId = brick->nodePos; nodeId < upperPos; nodeId++)
    {
        Node* node = &nodes[nodeId];
        node->delta = 0.0f;

        if(node->border >= 0.0f
                && node->potential > 0.01f)
        {
            synapseProcessing(&synapseSections[nodeId],
                              bricks,
                              nodes,
                              segmentMeta,
                              randomValues,
                              synapseMetaData,
                              networkMetaData,
                              node,
                              node->potential);
        }

        node->active = node->potential > node->border;
    }
}


#endif // SYNAPSE_PROCESSING_H
