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

/**
 * @brief outputSynapseProcessing
 * @param outputSection
 * @param outSectionPos
 * @return
 */
inline float
outputSynapseProcessing(OutputSegment* segment,
                        Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
                        OutputSynapseSection* outputSection)
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
            assert(targetId < segment->segmentMeta->numberOfInputs);
            synapse->active = segment->inputs[targetId] >= segment->outputMetaData->lowerMatch * synapse->border
                              && segment->inputs[targetId] <= segment->outputMetaData->upperMatch * synapse->border;
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
learNewOutput(OutputSegment* segment,
              Kitsunemimi::Ai::NetworkMetaData* networkMetaData,
              OutputSynapseSection* outputSection,
              const uint32_t outputPos)
{
    outputSection->newOnes = 0;
    int32_t toNew = static_cast<int32_t>(segment->outputMetaData->maxConnections)
                    - static_cast<int32_t>(outputSection->total);
    if(toNew <= 0) {
        return;
    }

    uint32_t pos = 0;
    while(pos < OUTPUT_SYNAPSES_PER_SECTION)
    {
        OutputSynapse* synapse = &outputSection->synapses[pos];

        if(synapse->targetId == UNINIT_STATE_32
                && networkMetaData->doLearn > 0)
        {
            // const uint32_t possibleTargetId = rand() % segment->segmentMeta->numberOfInputs;
            uint32_t possibleTargetId = rand() % segment->outputMetaData->inputRange;
            possibleTargetId += outputPos * segment->outputMetaData->inputOffset;
            assert(possibleTargetId <= segment->segmentMeta->numberOfInputs);
            if(segment->inputs[possibleTargetId] > 0.0f)
            {
                synapse->targetId = possibleTargetId;
                synapse->border = segment->inputs[possibleTargetId];
                synapse->weight = 0.0f;
                synapse->newOne = 1;
                synapse->active = 1;
            }
        }

        if(outputSection->newOnes == static_cast<uint32_t>(toNew)
                && synapse->newOne == 1)
        {
            outputSection->synapses[pos] = OutputSynapse();
        }

        if(synapse->newOne == 1) {
            outputSection->newOnes++;
            outputSection->total++;
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
                   Output* out)
{
    outputSection->diffNew = out->shouldValue - out->outputValue;
    outputSection->diffTotal = out->shouldValue - out->outputValue;

    if(out->shouldValue == 0.0f
            && out->outputValue <= out->shouldValue)
    {
        outputSection->diffNew = 0.0f;
        outputSection->diffTotal = 0.0f;
    }

    if(out->shouldValue > 0.0f
            && out->outputValue >= out->shouldValue)
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
output_node_processing(OutputSegment* segment,
                       Kitsunemimi::Ai::NetworkMetaData* networkMetaData)
{
    // process output
    for(uint32_t o = 0; o < segment->segmentMeta->numberOfOutputs; o++) {
        segment->outputs[o].outputValue = outputSynapseProcessing(segment,
                                                                  networkMetaData,
                                                                  &segment->outputSynapseSections[o]);
    }
}

/**
 * @brief output_learn_step
 * @return
 */
inline float
output_learn_step(OutputSegment* segment,
                  Kitsunemimi::Ai::NetworkMetaData* networkMetaData)
{
    float totalDiff = 0.0f;

    for(uint32_t o = 0; o < segment->segmentMeta->numberOfOutputs; o++)
    {
        learNewOutput(segment,
                      networkMetaData,
                      &segment->outputSynapseSections[o],
                      o);
        totalDiff += calculateLearnings(&segment->outputSynapseSections[o], &segment->outputs[o]);
        if(segment->outputSynapseSections[o].diffTotal != 0.0f)
        {
            outputSynapseLearn(&segment->outputSynapseSections[o]);
            segment->outputs[o].outputValue = outputSynapseProcessing(segment,
                                                                      networkMetaData,
                                                                      &segment->outputSynapseSections[o]);
        }
    }

    return totalDiff;
}

/**
 * @brief output_precheck
 * @return
 */
inline uint32_t
output_precheck(OutputSegment* segment)
{
    uint32_t updateVals = 0;

    for(uint32_t o = 0; o < segment->segmentMeta->numberOfOutputs; o++)
    {
        Output* out = &segment->outputs[o];
        if(out->shouldValue == 0.0f
                && out->outputValue <= out->shouldValue)
        {
            continue;
        }

        if(out->shouldValue > 0.0f
                && out->outputValue >= out->shouldValue)
        {
            continue;
        }

        updateVals++;
    }

    return updateVals;
}

#endif // OUTPUT_SYNAPSE_PROCESSING_H
