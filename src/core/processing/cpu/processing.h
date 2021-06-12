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
                  float* nodeBuffers,
                  CoreSegmentMeta* segmentMeta,
                  uint32_t* randomValues,
                  Kitsunemimi::Ai::CoreMetaData* synapseMetaData,
                  Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
                  Node* sourceNode,
                  const float weightIn)
{

    if(sourceNode->potential <= 0.01f) {
        return;
    }

    uint32_t counter = 0;

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
            netH -= static_cast<float>(synapse->border);
            nodeBuffers[synapse->targetNodeId] += outH * synapse->weight;

            synapse->activeCounter += (synapse->activeCounter < 126);
            counter = pos;
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
                const uint32_t startPoint,
                const uint32_t numberOfNodes)
{
    for(uint32_t i = startPoint; i < numberOfNodes + startPoint; i++)
    {
        // TODO: when port to gpu: change 2 to 256 again
        for(uint pos = 0; pos < 1; pos++)
        {
            const ulong nodeBufferPosition = (pos * (segmentMeta->numberOfNodes)) + i;
            nodes[i].currentState += nodeBuffers[nodeBufferPosition];
            nodeBuffers[nodeBufferPosition] = 0.0f;
        }
    }

    for(uint32_t i = startPoint; i < numberOfNodes + startPoint; i++)
    {
        Node* node = &nodes[i];
        node->delta = 0.0f;
        node->potential = synapseMetaData->potentialOverflow * node->currentState;

        if(node->border >= 0.0f)
        {
            synapseProcessing(&synapseSections[i],
                              bricks,
                              nodeBuffers,
                              segmentMeta,
                              randomValues,
                              synapseMetaData,
                              networkMetaData,
                              node,
                              node->potential);
        }

        node->active = node->currentState > node->border;
        node->currentState = 0.0f;
    }
}

#endif // SYNAPSE_PROCESSING_H
