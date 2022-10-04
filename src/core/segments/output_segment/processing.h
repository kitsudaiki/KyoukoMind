/**
 * @file        processing.h
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

#ifndef KYOUKOMIND_OUTPUT_PROCESSING_H
#define KYOUKOMIND_OUTPUT_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/segments/brick.h>
#include <core/cluster/cluster.h>

#include "objects.h"
#include "output_segment.h"

#include <io/hanami_messages.h>
#include <io/protobuf_messages.h>

/**
 * @brief get position of the highest output-position
 *
 * @param segment output-segment to check
 *
 * @return position of the highest output.
 */
inline uint32_t
getHighestOutput(const OutputSegment &segment)
{
    float hightest = -0.1f;
    uint32_t hightestPos = 0;
    OutputNeuron* out = nullptr;

    for(uint32_t outputNeuronId = 0;
        outputNeuronId < segment.segmentHeader->outputs.count;
        outputNeuronId++)
    {
        out = &segment.outputs[outputNeuronId];
        if(out->outputWeight > hightest)
        {
            hightest = out->outputWeight;
            hightestPos = outputNeuronId;
        }
    }

    return hightestPos;
}

/**
 * @brief process all neurons within a specific brick and also all synapse-sections,
 *        which are connected to an active neuron
 *
 * @param segment segment to process
 */
inline void
prcessOutputSegment(const OutputSegment &segment)
{
    OutputNeuron* neuron = nullptr;

    float* inputTransfers = segment.inputTransfers;
    for(uint64_t outputNeuronId = 0;
        outputNeuronId < segment.segmentHeader->outputs.count;
        outputNeuronId++)
    {
        neuron = &segment.outputs[outputNeuronId];
        if(neuron->shouldValue > neuron->maxWeight) {
            neuron->maxWeight = neuron->shouldValue;
        }

        neuron->outputWeight = inputTransfers[neuron->targetBorderId];
        neuron->outputWeight = 1.0f / (1.0f + exp(-1.0f * neuron->outputWeight));
        neuron->shouldValue /= neuron->maxWeight;
    }

    // send output back if a client-connection is set
    if(segment.parentCluster->msgClient != nullptr
            && segment.parentCluster->mode == Cluster::NORMAL_MODE)
    {
        if(segment.parentCluster->useProtobuf) {
            sendProtobufOutputMessage(segment);
        } else {
            sendHanamiOutputMessage(segment);
        }
    }
}


#endif // KYOUKOMIND_OUTPUT_PROCESSING_H
