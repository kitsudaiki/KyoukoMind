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
inline float
outputSynapseProcessing(OutputSynapseSection* outputSection)
{
    uint32_t pos = 0;
    float outputWeight = 0.0f;
    float* transferNodes = Kitsunemimi::getBuffer<float>(KyoukoRoot::m_segment->transferNodeBuffer);

    while(pos < 224)
    {
        OutputSynapse* synapse = &outputSection->synapses[pos];
        const uint32_t targetId = synapse->targetId;

        if(targetId != UNINIT_STATE_32)
        {
            const float weight = transferNodes[targetId];
            const uint32_t active = weight >= synapse->border;
            outputWeight += synapse->weight * static_cast<float>(active);
        }

        pos++;
    }

    return outputWeight;
}

/**
 * @brief outputSynapseLearn
 * @param sectionPos
 * @param diff
 * @return
 */
inline bool
outputSynapseLearn(OutputSynapseSection* outputSection,
                   const float outputDiff)
{
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    float* transferNodes = Kitsunemimi::getBuffer<float>(KyoukoRoot::m_segment->transferNodeBuffer);

    float learnWeight = fabs(outputDiff);

    while(learnWeight > 0.0f)
    {
        float partWeight = 10.0f;
        if(learnWeight < partWeight) {
            partWeight = learnWeight;
        }

        // update target
        OutputSynapse* synapse = &outputSection->synapses[rand() % 224];
        if(synapse->targetId == UNINIT_STATE_32)
        {
            synapse->targetId = rand() % globalValue->nodesPerBrick;
            synapse->border = transferNodes[synapse->targetId];
        }

        // update weight
        if(outputDiff < 0.0f) {
            synapse->weight += -1.0f * partWeight;
        } else {
            synapse->weight += partWeight;
        }

        learnWeight -= partWeight;
    }

    return false;
}

/**
 * @brief node_processing
 */
inline bool
output_node_processing()
{
    Output* outputs = Kitsunemimi::getBuffer<Output>(KyoukoRoot::m_segment->outputs);
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    Kitsunemimi::ItemBuffer* buf = &KyoukoRoot::m_segment->outputSynapses;
    OutputSynapseSection* outputSection = Kitsunemimi::getBuffer<OutputSynapseSection>(*buf);

    const uint64_t numberOutputs = KyoukoRoot::m_segment->outputSynapses.numberOfItems;

    // process output
    for(uint32_t o = 0; o < numberOutputs; o++) {
        outputs[o].outputValue = outputSynapseProcessing(&outputSection[o]);
    }

    // learn new
    if(globalValue->doLearn > 0)
    {
        bool result = false;
        for(uint32_t o = 0; o < numberOutputs; o++)
        {
            const float diff = outputs[o].shouldValue - outputs[o].outputValue;
            result |= outputSynapseLearn(&outputSection[o], diff);
            outputs[o].outputValue = outputSynapseProcessing(&outputSection[o]);
        }
        return result;
    }

    return true;
}

#endif // OUTPUT_SYNAPSE_PROCESSING_H
