/**
 * @file        output_synapse_processing.h
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

#ifndef OUTPUT_SYNAPSE_PROCESSING_H
#define OUTPUT_SYNAPSE_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/output.h>
#include <core/objects/segment.h>
#include <core/objects/synapses.h>
#include <core/objects/global_values.h>

#include <libKitsunemimiCommon/buffer/item_buffer.h>

/**
 * @brief outputSynapseProcessing
 * @param sectionPos
 * @param weight
 * @return
 */
inline void
outputSynapseProcessing(const uint32_t sectionPos,
                        float weight)
{
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    Kitsunemimi::ItemBuffer* buf = &KyoukoRoot::m_segment->outputSynapses;
    OutputSynapseSection* synapseSections = &Kitsunemimi::getBuffer<OutputSynapseSection>(*buf)[sectionPos];
    Output* outputs = Kitsunemimi::getBuffer<Output>(KyoukoRoot::m_segment->outputs);

    uint32_t pos = 0;

    // iterate over all synapses in the section and update the target-nodes
    while(pos < 255)
    {
        OutputSynapse* synapse = &synapseSections->synapses[pos];

        if(weight > 0.0f)
        {
            if(synapse->targetNodeId == UNINIT_STATE_16
                    && globalValue->doLearn > 0)
            {
                synapse->hardening = 0.0f;
                synapse->weightIn = 10.0f;
                synapse->weightOut = 0.0f;
                synapse->newOne = 1;
                synapse->weightOut *= static_cast<float>(1 - (rand() % 2) * 2);
                synapse->targetNodeId = static_cast<uint16_t>(rand() % 0xFFFF)
                                        % KyoukoRoot::m_segment->outputs.numberOfItems;
            }

            if(synapse->targetNodeId != UNINIT_STATE_16)
            {
                if(synapse->newOne == 1) {
                    outputs[synapse->targetNodeId].newOnes++;
                }

                float newHardening = synapse->hardening + globalValue->lerningValue;
                newHardening = (newHardening > 1.0f) * 1.0f + (newHardening <= 1.0f) * newHardening;
                synapse->hardening = newHardening;
                if(synapse->hardening >= 0.99f) {
                    synapse->newOne = 0;
                }

                float ratio = weight / synapse->weightIn;
                ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;
                outputs[synapse->targetNodeId].outputValue += ratio * synapse->weightOut;

                weight -= synapse->weightIn;
            }
        }
        else
        {
            if(synapse->newOne == 1)
            {
                synapse->targetNodeId = UNINIT_STATE_16;
                synapse->weightOut = 0.0f;
                synapse->newOne = 0;
            }
        }

        pos++;
    }
}

/**
 * @brief outputSynapseLearn
 * @param sectionPos
 * @param diff
 * @return
 */
inline void
outputSynapseLearn(const uint32_t sectionPos,
                   float weight)
{
    Kitsunemimi::ItemBuffer* buf = &KyoukoRoot::m_segment->outputSynapses;
    OutputSynapseSection* synapseSections = &Kitsunemimi::getBuffer<OutputSynapseSection>(*buf)[sectionPos];
    Output* outputs = Kitsunemimi::getBuffer<Output>(KyoukoRoot::m_segment->outputs);
    uint32_t pos = 0;

    // iterate over all synapses in the section and update the target-nodes
    while(pos < 255
          && weight > 0.0f)
    {
        OutputSynapse* synapse = &synapseSections->synapses[pos];

        if(synapse->newOne == 1
                && synapse->targetNodeId != UNINIT_STATE_16)
        {
            if(outputs[synapse->targetNodeId].newOnes > 0) {
                synapse->weightOut += outputs[synapse->targetNodeId].diff;
            }
        }

        weight -= synapse->weightIn;
        pos++;
    }
}

inline void
calculateLearnings(float &totalDiff,
                   uint32_t &newOnes,
                   const bool resetOutput)
{
    totalDiff = 0.0f;
    newOnes = 0;

    Output* outputs = Kitsunemimi::getBuffer<Output>(KyoukoRoot::m_segment->outputs);
    const uint64_t outputsSize = KyoukoRoot::m_segment->outputs.numberOfItems;

    for(uint64_t o = 0; o < outputsSize; o++)
    {
        outputs[o].diff = outputs[o].shouldValue - outputs[o].outputValue;
        totalDiff += fabs(outputs[o].diff);
        newOnes += outputs[o].newOnes;
        outputs[o].diff /= static_cast<float>(outputs[o].newOnes);

        if(resetOutput) {
            outputs[o].outputValue = 0.0f;
        }
    }
}

/**
 * @brief node_processing
 */
void
output_node_processing()
{
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    float* outputNodes = Kitsunemimi::getBuffer<float>(KyoukoRoot::m_segment->nodeOutputBuffer);
    Output* outputs = Kitsunemimi::getBuffer<Output>(KyoukoRoot::m_segment->outputs);

    const uint64_t outputBufferSize = KyoukoRoot::m_segment->nodeOutputBuffer.numberOfItems;
    const uint64_t outputsSize = KyoukoRoot::m_segment->outputs.numberOfItems;

    // reset output-values
    for(uint64_t o = 0; o < outputsSize; o++)
    {
        outputs[o].outputValue = 0.0f;
        outputs[o].newOnes = 0;
    }

    // process output
    for(uint32_t i = 0; i < outputBufferSize; i++) {
        /*if(outputNodes[i] > 0.0f) {
            std::cout<<"outputNodes[i]: "<<outputNodes[i]<<std::endl;
        }*/
        outputSynapseProcessing(i, outputNodes[i]);
    }

    if(globalValue->doLearn != 0)
    {
        float totalDiff = 0.0f;
        uint32_t newOnes = 0;
        std::cout<<"learn"<<std::endl;

        std::cout<<"totalDiff1: "<<totalDiff<<std::endl;
        std::cout<<"newOnes1: "<<newOnes<<std::endl;

        calculateLearnings(totalDiff, newOnes, true);

        // learn and rerun output-processing
        for(uint32_t i = 0; i < outputBufferSize; i++)
        {
            outputSynapseLearn(i, outputNodes[i]);
            outputSynapseProcessing(i, outputNodes[i]);
        }

        calculateLearnings(totalDiff, newOnes, false);

        if(totalDiff < 0.001f)
        {
            for(uint32_t i = 0; i < 3; i++) {
                outputs[i].shouldValue = 0.0f;
            }
            KyoukoRoot::m_freezeState = true;
            globalValue->doLearn = 0;
        }
        std::cout<<"totalDiff2: "<<totalDiff<<std::endl;
        std::cout<<"newOnes2: "<<newOnes<<std::endl;
    }
}

#endif // OUTPUT_SYNAPSE_PROCESSING_H
