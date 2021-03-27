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
 * @param outputSection
 * @param outSectionPos
 * @return
 */
inline float
outputSynapseProcessing(OutputSynapseSection* outputSection)
{
    float outputWeight = 0.0f;
    float* transferNodes = Kitsunemimi::getBuffer<float>(KyoukoRoot::m_segment->transferNodeBuffer);
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    outputSection->total = 0;

    uint32_t pos = 0;
    while(pos < OUTPUT_SYNAPSES_PER_SECTION)
    {
        OutputSynapse* synapse = &outputSection->synapses[pos];

        if(globalValue->lerningValue > 0.0f) {
            synapse->newOne = 0;
        }

        const uint32_t targetId = synapse->targetId;
        if(targetId != UNINIT_STATE_32)
        {
            assert(targetId < globalValue->nodesPerBrick);
            synapse->active = transferNodes[targetId] >=  1.0f * synapse->border
                              && transferNodes[targetId] <= 2.0f * synapse->border;
            outputWeight += synapse->weight * static_cast<float>(synapse->active);
            outputSection->total += synapse->active;
        }

        pos++;
    }

    return outputWeight;
}

/**
 * @brief learNewOutput
 * @param outputSection
 * @return
 */
inline void
learNewOutput(OutputSynapseSection* outputSection)
{
    float* transferNodes = Kitsunemimi::getBuffer<float>(KyoukoRoot::m_segment->transferNodeBuffer);
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    outputSection->newOnes = 0;

    uint32_t pos = 0;
    while(pos < OUTPUT_SYNAPSES_PER_SECTION)
    {
        OutputSynapse* synapse = &outputSection->synapses[pos];

        if(synapse->targetId == UNINIT_STATE_32
                && globalValue->doLearn > 0)
        {
            const uint32_t possibleTargetId = rand() % globalValue->nodesPerBrick;
            if(transferNodes[possibleTargetId] > 0.0f)
            {
                synapse->targetId = possibleTargetId;
                synapse->border = transferNodes[possibleTargetId];
                synapse->weight = 0.0f;
                synapse->newOne = 1;
            }
        }

        if(outputSection->newOnes == 10
                && synapse->newOne == 1)
        {
            outputSection->synapses[pos] = OutputSynapse();
        }

        if(synapse->newOne == 1) {
            outputSection->newOnes++;
        }

        pos++;
    }
}

/**
 * @brief outputSynapseLearn
 * @param outputSection
 * @return
 */
inline void
outputSynapseLearn(OutputSynapseSection* outputSection)
{
    uint32_t pos = 0;
    while(pos < OUTPUT_SYNAPSES_PER_SECTION)
    {
        // update target
        OutputSynapse* synapse = &outputSection->synapses[pos];
        if(synapse->targetId != UNINIT_STATE_32)
        {
            if(synapse->newOne == 1) {
                synapse->weight += outputSection->diffNew;
            }

            if(outputSection->newOnes == 0
                    && synapse->active > 0)
            {
                synapse->weight += outputSection->diffTotal;
            }
        }

        pos++;
    }
}

/**
 * @brief calculateLearnings
 * @param outputSection
 * @param out
 * @return
 */
inline float
calculateLearnings(OutputSynapseSection* outputSection,
                   Output* out)
{
    outputSection->diffNew = out->shouldValue - out->outputValue;
    outputSection->diffTotal = out->shouldValue - out->outputValue;

    const float totalDiff = fabs(outputSection->diffNew);

    outputSection->diffNew /= static_cast<float>(outputSection->newOnes + 1);
    outputSection->diffTotal /= static_cast<float>(outputSection->total + 1);

    return totalDiff;
}

/**
 * @brief node_processing
 */
inline void
output_node_processing()
{
    Output* outputs = Kitsunemimi::getBuffer<Output>(KyoukoRoot::m_segment->outputs);
    Kitsunemimi::ItemBuffer* buf = &KyoukoRoot::m_segment->outputSynapses;
    OutputSynapseSection* outputSection = Kitsunemimi::getBuffer<OutputSynapseSection>(*buf);

    // process output
    const uint64_t numberOutputs = KyoukoRoot::m_segment->outputSynapses.numberOfItems;
    for(uint32_t o = 0; o < numberOutputs; o++) {
        outputs[o].outputValue = outputSynapseProcessing(&outputSection[o]);
    }
}

/**
 * @brief output_learn_step
 * @return
 */
inline float
output_learn_step()
{
    Output* outputs = Kitsunemimi::getBuffer<Output>(KyoukoRoot::m_segment->outputs);
    Kitsunemimi::ItemBuffer* buf = &KyoukoRoot::m_segment->outputSynapses;
    OutputSynapseSection* outputSection = Kitsunemimi::getBuffer<OutputSynapseSection>(*buf);

    output_node_processing();
    float totalDiff = 0.0f;

    const uint64_t numberOutputs = KyoukoRoot::m_segment->outputSynapses.numberOfItems;
    for(uint32_t o = 0; o < numberOutputs; o++)
    {
        learNewOutput(&outputSection[o]);
        totalDiff += calculateLearnings(&outputSection[o], &outputs[o]);
        outputSynapseLearn(&outputSection[o]);
        outputs[o].outputValue = outputSynapseProcessing(&outputSection[o]);
    }

    return totalDiff;
}

/**
 * @brief resetNewOnes
 */
inline void
resetNewOnes()
{
    Kitsunemimi::ItemBuffer* buf = &KyoukoRoot::m_segment->outputSynapses;
    OutputSynapseSection* outputSection = Kitsunemimi::getBuffer<OutputSynapseSection>(*buf);

    const uint64_t numberOutputs = KyoukoRoot::m_segment->outputSynapses.numberOfItems;
    for(uint32_t o = 0; o < numberOutputs; o++)
    {
        for(uint32_t i = 0; i < OUTPUT_SYNAPSES_PER_SECTION; i++)
        {
            if(outputSection->synapses[i].newOne == 1) {
                outputSection->synapses[i] = OutputSynapse();
            }
        }
    }
}

#endif // OUTPUT_SYNAPSE_PROCESSING_H
