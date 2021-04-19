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
#include <core/objects/network_cluster.h>

#include <libKitsunemimiAiCommon/metadata.h>

/**
 * @brief outputSynapseProcessing
 * @param outputSection
 * @param outSectionPos
 * @return
 */
inline float
outputSynapseProcessing(OutputSynapseSection* outputSection,
                        float* inputs,
                        OutputSegmentMeta* segmentMeta,
                        Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
                        Kitsunemimi::Ai::OutputMetaData* outputMetaData)
{
    float outputWeight = 0.0f;

    outputSection->total = 0;

    uint32_t pos = 0;
    while(pos < OUTPUT_SYNAPSES_PER_SECTION)
    {
        OutputSynapse* synapse = &outputSection->synapses[pos];

        if(networkMetaData->lerningValue > 0.0f)
        {
            synapse->newOne = 0;
            if(synapse->weight == 0.0f)
            {
                outputSection->synapses[pos] = OutputSynapse();
                pos++;
                continue;
            }
        }

        const uint32_t targetId = synapse->targetId;
        if(targetId != UNINIT_STATE_32)
        {
            assert(targetId < segmentMeta->numberOfInputs);
            synapse->active = inputs[targetId] >= outputMetaData->lowerMatch * synapse->border
                              && inputs[targetId] <= outputMetaData->upperMatch * synapse->border;
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
learNewOutput(OutputSynapseSection* section,
              float* inputs,
              OutputSegmentMeta* segmentMeta,
              uint32_t* randomValues,
              Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
              Kitsunemimi::Ai::OutputMetaData* outputMetaData,
              const uint32_t outputPos)
{
    section->newOnes = 0;
    int32_t toNew = static_cast<int32_t>(outputMetaData->maxConnections)
                    - static_cast<int32_t>(section->total);
    if(toNew <= 0) {
        return;
    }

    uint32_t limiter = 0;
    uint32_t pos = 0;
    while(pos < OUTPUT_SYNAPSES_PER_SECTION)
    {
        OutputSynapse* synapse = &section->synapses[pos];

        if(synapse->targetId == UNINIT_STATE_32
                && networkMetaData->doLearn > 0
                && limiter < 3)
        {
            // const uint32_t possibleTargetId = rand() % segment->segmentMeta->numberOfInputs;
            section->randomPos = (section->randomPos + 1) % segmentMeta->numberOfRandomValues;
            uint32_t possibleTargetId = randomValues[section->randomPos] % outputMetaData->inputRange;
            possibleTargetId += outputPos * outputMetaData->inputOffset;
            assert(possibleTargetId <= segmentMeta->numberOfInputs);
            if(inputs[possibleTargetId] > 0.0f)
            {
                synapse->targetId = possibleTargetId;
                synapse->border = inputs[possibleTargetId];
                synapse->weight = 0.0f;
                synapse->newOne = 1;
                synapse->active = 1;
                limiter++;
            }
        }

        if(section->newOnes == static_cast<uint32_t>(toNew)
                && synapse->newOne == 1)
        {
            section->synapses[pos] = OutputSynapse();
        }

        if(synapse->newOne == 1) {
            section->newOnes++;
            section->total++;
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
            if(synapse->active > 0)
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
                   Output* output)
{
    outputSection->diffNew = output->shouldValue - output->outputValue;
    outputSection->diffTotal = output->shouldValue - output->outputValue;

    if(output->shouldValue == 0.0f
            && output->outputValue <= output->shouldValue)
    {
        outputSection->diffNew = 0.0f;
        outputSection->diffTotal = 0.0f;
    }

    if(output->shouldValue > 0.0f
            && output->outputValue >= output->shouldValue)
    {
        outputSection->diffNew = 0.0f;
        outputSection->diffTotal = 0.0f;
    }

    const float totalDiff = fabs(outputSection->diffNew);

    outputSection->diffNew /= static_cast<float>(outputSection->newOnes + 1);
    outputSection->diffTotal /= static_cast<float>(outputSection->total + 1);

    return totalDiff;
}

/**
 * @brief node_processing
 */
inline void
output_node_processing(OutputSynapseSection* outputSynapseSections,
                       float* inputs,
                       Output* outputs,
                       OutputSegmentMeta* segmentMeta,
                       Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
                       Kitsunemimi::Ai::OutputMetaData* outputMetaData)
{
    // process output
    for(uint32_t o = 0; o < segmentMeta->numberOfOutputs; o++)
    {
        outputs[o].outputValue = outputSynapseProcessing(&outputSynapseSections[o],
                                                         inputs,
                                                         segmentMeta,
                                                         networkMetaData,
                                                         outputMetaData);
    }
}

/**
 * @brief output_learn_step
 * @return
 */
inline void
output_learn_step(OutputSynapseSection* outputSynapseSections,
                  float* inputs,
                  Output* outputs,
                  OutputSegmentMeta* segmentMeta,
                  uint32_t* randomValues,
                  Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
                  Kitsunemimi::Ai::OutputMetaData* outputMetaData)
{
    for(uint32_t o = 0; o < segmentMeta->numberOfOutputs; o++)
    {
        learNewOutput(&outputSynapseSections[o],
                      inputs,
                      segmentMeta,
                      randomValues,
                      networkMetaData,
                      outputMetaData,
                      o);
        calculateLearnings(&outputSynapseSections[o], &outputs[o]);
        if(outputSynapseSections[o].diffTotal != 0.0f)
        {
            outputSynapseLearn(&outputSynapseSections[o]);
            outputs[o].outputValue = outputSynapseProcessing(&outputSynapseSections[o],
                                                             inputs,
                                                             segmentMeta,
                                                             networkMetaData,
                                                             outputMetaData);
        }
    }
}

#endif // OUTPUT_SYNAPSE_PROCESSING_H
