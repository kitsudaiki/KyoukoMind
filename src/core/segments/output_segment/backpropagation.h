/**
 * @file        backpropagation.h
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

#ifndef KYOUKOMIND_OUTPUT_BACKPROPAGATION_H
#define KYOUKOMIND_OUTPUT_BACKPROPAGATION_H

#include <common.h>

#include <kyouko_root.h>
#include <core/segments/brick.h>
#include <core/cluster/cluster.h>

#include "objects.h"
#include "output_segment.h"

/**
 * @brief backpropagate output
 *
 * @param segment pointer to currect output-segment to process
 */
inline void
backpropagateOutput(const OutputSegment &segment)
{
    OutputNeuron out;
    float delta = 0.0f;

    // iterate over all output-neurons
    for(uint64_t outputNeuronId = 0;
        outputNeuronId < segment.segmentHeader->outputs.count;
        outputNeuronId++)
    {
        out = segment.outputs[outputNeuronId];
        delta = (out.outputWeight - out.shouldValue);
        delta *= out.outputWeight * (1.0f - out.outputWeight);
        segment.outputTransfers[out.targetBorderId] = delta;
    }
}

#endif // KYOUKOMIND_OUTPUT_BACKPROPAGATION_H
