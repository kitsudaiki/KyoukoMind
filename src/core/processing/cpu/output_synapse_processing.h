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
#include <core/objects/item_buffer.h>
#include <core/objects/synapses.h>
#include <core/objects/global_values.h>

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
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    ItemBuffer* buf = &KyoukoRoot::m_segment->outputSynapses;
    OutputSynapseSection* synapseSections = &getBuffer<OutputSynapseSection>(*buf)[sectionPos];
    Output* outputs = getBuffer<Output>(KyoukoRoot::m_segment->outputs);

    uint32_t pos = 0;

    // iterate over all synapses in the section and update the target-nodes
    while(pos < 255
          && weight > 0.0f)
    {
        OutputSynapse* synapse = &synapseSections->synapses[pos];

        if(synapse->targetNodeId == UNINIT_STATE_16)
        {
            if(globalValue->doLearn == 0) {
                return;
            }

            synapse->hardening = 0.0f;

            // set new weight
            synapse->weightIn = 10.0f;
            synapse->weightOut = 0.0f;
            synapse->weightOut *= static_cast<float>(1 - (rand() % 2) * 2);
            synapse->targetNodeId = static_cast<uint16_t>(rand() % 0xFFFF)
                                    % KyoukoRoot::m_segment->outputs.numberOfItems;
            synapse->newOne = 1;
        }

        if(synapse->newOne == 1) {
            outputs[synapse->targetNodeId].newOnes++;
        }

        float newHardening = synapse->hardening + globalValue->lerningValue;
        newHardening = (newHardening > 1.0f) * 1.0f + (newHardening <= 1.0f) * newHardening;
        synapse->hardening = newHardening;
        if(synapse->hardening == 1.0f) {
            synapse->newOne = 0;
        }

        float ratio = weight / synapse->weightIn;
        ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;
        outputs[synapse->targetNodeId].outputValue += ratio * synapse->weightOut;

        weight -= synapse->weightIn;
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
    ItemBuffer* buf = &KyoukoRoot::m_segment->outputSynapses;
    OutputSynapseSection* synapseSections = &getBuffer<OutputSynapseSection>(*buf)[sectionPos];
    Output* outputs = getBuffer<Output>(KyoukoRoot::m_segment->outputs);
    uint32_t pos = 0;

    // iterate over all synapses in the section and update the target-nodes
    while(pos < 255
          && weight > 0.0f)
    {
        OutputSynapse* synapse = &synapseSections->synapses[pos];

        if(synapse->newOne == 1
                && synapse->targetNodeId != UNINIT_STATE_16)
        {
            if(outputs[synapse->targetNodeId].newOnes > 0)
            {
                synapse->weightOut += outputs[synapse->targetNodeId].diff;
                synapse->newOne = 0;
            }
        }

        weight -= synapse->weightIn;
        pos++;
    }
}

/**
 * @brief outputSynapseReset
 * @param sectionPos
 */
inline void
outputSynapseReset(const uint32_t sectionPos)
{
    ItemBuffer* buf = &KyoukoRoot::m_segment->outputSynapses;
    OutputSynapseSection* synapseSections = &getBuffer<OutputSynapseSection>(*buf)[sectionPos];

    uint32_t pos = 0;

    // iterate over all synapses in the section and update the target-nodes
    while(pos < 255)
    {
        OutputSynapse* synapse = &synapseSections->synapses[pos];
        synapse->newOne = 0.0f;
        pos++;
    }
}


/**
 * @brief node_processing
 */
void
output_node_processing()
{
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    float* outputNodes = getBuffer<float>(KyoukoRoot::m_segment->nodeOutputBuffer);
    Output* outputs = getBuffer<Output>(KyoukoRoot::m_segment->outputs);

    const uint64_t outputBufferSize = KyoukoRoot::m_segment->nodeOutputBuffer.numberOfItems;
    const uint64_t outputsSize = KyoukoRoot::m_segment->outputs.numberOfItems;

    // reset output-values
    for(uint64_t i = 0; i < outputsSize; i++)
    {
        outputs[i].outputValue = 0.0f;
        outputs[i].newOnes = 0;
    }

    float total = 0.0f;
    float totalAbs = 0.0f;

    // process output
    for(uint32_t i = 0; i < outputBufferSize; i++) {
        outputSynapseProcessing(i, outputNodes[i]);
        total += outputNodes[i];
        totalAbs += fabs(outputNodes[i]);
    }

    std::cout<<"total: "<<total<<std::endl;
    std::cout<<"totalAbs: "<<totalAbs<<std::endl;

    if(globalValue->doLearn != 0)
    {
        float totalDiff = 0.0f;
        uint32_t newOnes = 0;
        std::cout<<"learn"<<std::endl;
        Output tempOutput[3];

        // calc values for learning
        for(uint64_t i = 0; i < outputsSize; i++)
        {
            outputs[i].diff = outputs[i].shouldValue - outputs[i].outputValue;
            totalDiff += fabs(outputs[i].diff);
            newOnes += outputs[i].newOnes;
            outputs[i].diff /= static_cast<float>(outputs[i].newOnes) + 0.0000001f;
            tempOutput[i] = outputs[i];
            outputs[i].outputValue = 0.0f;
        }


        std::cout<<"totalDiff1: "<<totalDiff<<std::endl;
        std::cout<<"newOnes1: "<<newOnes<<std::endl;

        // learn and rerun output-processing
        for(uint32_t i = 0; i < outputBufferSize; i++)
        {
            outputSynapseLearn(i, outputNodes[i]);
            outputSynapseProcessing(i, outputNodes[i]);
        }

        // calc values again after lerning
        totalDiff = 0.0f;
        newOnes = 0;
        for(uint64_t i = 0; i < outputsSize; i++)
        {
            outputs[i].diff = outputs[i].shouldValue - outputs[i].outputValue;
            totalDiff += fabs(outputs[i].diff);
            outputs[i].diff /= static_cast<float>(outputs[i].newOnes) + 0.0000001f;
            tempOutput[i] = outputs[i];

            newOnes += outputs[i].newOnes;
        }
        std::cout<<"totalDiff2: "<<totalDiff<<std::endl;
        std::cout<<"newOnes2: "<<newOnes<<std::endl;
    }

    for(uint32_t i = 0; i < outputBufferSize; i++) {
        outputSynapseReset(i);
    }
}

#endif // OUTPUT_SYNAPSE_PROCESSING_H
