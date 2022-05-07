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

#ifndef KYOUKOMIND_INPUT_PROCESSING_H
#define KYOUKOMIND_INPUT_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/segments/brick.h>

#include "objects.h"
#include "input_segment.h"

/**
 * @brief process all nodes within a specific brick and also all synapse-sections,
 *        which are connected to an active node
 *
 * @param segment input-segment to process
 */
void
prcessInputSegment(const InputSegment &segment)
{
    InputNode* node = nullptr;
    const uint64_t numberOfInputs = segment.segmentHeader->inputs.count;
    float* outputTransfers = segment.outputTransfers;

    for(uint64_t pos = 0; pos < numberOfInputs; pos++)
    {
        node = &segment.inputs[pos];

        node->maxWeight = static_cast<float>(node->maxWeight >= node->weight) * node->maxWeight
                          + static_cast<float>(node->maxWeight < node->weight) * node->weight;
        //outputTransfers[node->targetBorderId] = node->weight / node->maxWeight;

        node->weight *= 5.0f;
        if(node->weight > 1.0f) {
            node->weight = 1.0f;
        }

        outputTransfers[node->targetBorderId] = node->weight;
    }
    //std::cout<<"###########################################################"<<std::endl;
    //std::cout<<segment.inputs[0].weight<<"   :   "<<segment.inputs[1].weight<<std::endl;
    //std::cout<<segment.inputs[2].weight<<"   :   "<<segment.inputs[3].weight<<std::endl;
    //std::cout<<segment.inputs[4].weight<<"   :   "<<segment.inputs[5].weight<<std::endl;
    //std::cout<<segment.inputs[6].weight<<"   :   "<<segment.inputs[7].weight<<std::endl;

}

#endif // KYOUKOMIND_INPUT_PROCESSING_H
