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
                        OutputInput* inputs,
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

        if(networkMetaData->lerningValue > 0.0f) {
            synapse->newOne = 0;
        }

        if(synapse->weight != 0.0f)
        {
            const uint targetId = synapse->targetId;
            if(targetId != UNINIT_STATE_32)
            {
                synapse->active = inputs[targetId].weight >= outputMetaData->lowerMatch * synapse->border
                                  && inputs[targetId].weight <= outputMetaData->upperMatch * synapse->border;
                outputWeight += synapse->weight * static_cast<float>(synapse->active);
                outputSection->total += synapse->active;
            }
        }
        else
        {
            synapse->valid = 0;
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
              OutputInput* inputs,
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
        if(synapse->valid == 0
                && networkMetaData->doLearn > 0
                && limiter < 5)
        {
            // const uint32_t possibleTargetId = rand() % segment->segmentMeta->numberOfInputs;
            section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
            uint32_t possibleTargetId = randomValues[section->randomPos] % outputMetaData->inputRange;
            possibleTargetId += outputPos * outputMetaData->inputOffset;

            const uint8_t ok = inputs[possibleTargetId].weight > 0.0f && inputs[possibleTargetId].isNew == 1;
            synapse->targetId = possibleTargetId;
            synapse->border = inputs[possibleTargetId].weight;
            synapse->weight = 0.0f;
            synapse->newOne = ok;
            synapse->active = ok;
            synapse->valid = ok;
            limiter += ok;

        }

        const uint8_t reset = (section->newOnes == static_cast<uint32_t>(toNew) && synapse->newOne == 1) == false;
        synapse->valid = reset * synapse->valid;

        const uint8_t updateVal = synapse->newOne == 1;
        section->newOnes += updateVal;
        section->total += updateVal;

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
            if(synapse->active > 0) {
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
inline void
calculateLearnings(OutputSynapseSection* outputSection,
                   Output* output)
{
    outputSection->diffTotal = output->shouldValue - output->outputValue;

    bool good1Bool = output->shouldValue == 0.0f && output->outputValue <= output->shouldValue;
    good1Bool = good1Bool || (output->shouldValue > 0.0f && output->outputValue >= output->shouldValue);
    outputSection->diffTotal *= static_cast<float>(good1Bool == false);
    outputSection->diffTotal /= static_cast<float>(outputSection->total + 1);
}


/**
 * @brief node_processing
 */
inline void
output_node_processing(OutputSynapseSection* outputSynapseSections,
                       OutputInput* inputs,
                       Output* outputs,
                       OutputSegmentMeta* segmentMeta,
                       Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
                       Kitsunemimi::Ai::OutputMetaData* outputMetaData,
                       const uint32_t threadId,
                       const uint32_t numberOfThreads)
{
    // process output
    for(uint32_t o = threadId; o < segmentMeta->numberOfOutputs; o = o + numberOfThreads)
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
                  OutputInput* inputs,
                  Output* outputs,
                  OutputSegmentMeta* segmentMeta,
                  uint32_t* randomValues,
                  Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
                  Kitsunemimi::Ai::OutputMetaData* outputMetaData,
                  const uint32_t threadId,
                  const uint32_t numberOfThreads)
{
    for(uint32_t o = threadId; o < segmentMeta->numberOfOutputs; o = o + numberOfThreads)
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
