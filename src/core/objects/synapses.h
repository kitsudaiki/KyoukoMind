/**
 * @file        kyouko_root.cpp
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

#ifndef SYNAPSES_H
#define SYNAPSES_H

#include <common.h>

//==================================================================================================

struct Synapse
{
    float staticWeight = 0.0;
    float dynamicWeight = 0.0;
    float hardening = 0.0f;
    uint16_t targetNodeId = UNINIT_STATE_16;
    int8_t sign = 1;
    uint8_t padding[1];
    // total size: 16 Byte
};

//==================================================================================================

struct SynapseSection
{
    uint8_t status = ACTIVE_SECTION;
    uint8_t positionInEdge = UNINIT_STATE_8;

    uint16_t randomPos = UNINIT_STATE_16;

    uint32_t sourceEdgeId = UNINIT_STATE_32;
    uint32_t sourceBrickId = UNINIT_STATE_32;

    // has to be at least a very small value to avoid division by zero
    float totalWeight = 0.0000001f;

    Synapse synapses[SYNAPSES_PER_SYNAPSESECTION];


    SynapseSection()
    {
        for(uint32_t i = 0; i < SYNAPSES_PER_SYNAPSESECTION; i++)
        {
            Synapse newSynapse;
            synapses[i] = newSynapse;
        }
    }
    // total size: 256 Byte
};

//==================================================================================================

#endif // SYNAPSES_H
