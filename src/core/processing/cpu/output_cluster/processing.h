﻿/**
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

#ifndef OUTPUT_PROCESSING_H
#define OUTPUT_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segments/output_segment.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

/**
 * @brief process all nodes within a specific brick and also all synapse-sections,
 *        which are connected to an active node
 *
 * @param segment segment to process
 */
void
prcessSegment(OutputSegment* segment)
{
    const uint32_t numberOfOutputs = segment->segmentHeader->outputs.count;
    float* inputTransfers = segment->inputTransfers;
    for(uint32_t pos = 0; pos < numberOfOutputs; pos++)
    {
        OutputNode* node = &segment->outputs[pos];
        node->outputWeight = inputTransfers[node->targetBorderId];
    }
}

#endif // OUTPUT_PROCESSING_H
