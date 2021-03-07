/**
 * @file        global_values.h
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

#ifndef GLOBAL_VALUES_HANDLER_H
#define GLOBAL_VALUES_HANDLER_H

#include <common.h>
#include <libKitsunemimiCommon/buffer/item_buffer.h>

struct GlobalValues
{
    uint32_t status = Kitsunemimi::ItemBuffer::ACTIVE_SECTION;

    // init
    uint16_t nodesPerBrick = 0;
    uint16_t maxBrickDistance = 0;
    float nodeLowerBorder = 0.0f;
    float nodeUpperBorder = 0.0f;
    uint64_t maxSynapseSections = 0;
    uint32_t layer = 0;

    // processing
    float lerningValue = 0.0f;
    float synapseDeleteBorder = 1.0f;
    float actionPotential = 100.0f;
    float nodeCooldown = 100.0f;
    float memorizing = 0.1f;
    float gliaValue = 1.0f;
    float signNeg = 0.6f;
    float potentialOverflow = 20.0f;
    float maxSynapseWeight = 30.0f;
    uint32_t cycleTime = 1000000;
    float inputFlowGradiant = 0.1f;
    float nodeFlowGradiant = 0.0001f;
    uint8_t refractionTime = 1;

    uint8_t doLearn = 0;
};

#endif // GLOBAL_VALUES_HANDLER_H
