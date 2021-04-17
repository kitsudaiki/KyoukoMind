/**
 * @file        synapses.h
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

#include <libKitsunemimiCommon/buffer/item_buffer.h>

//==================================================================================================

struct SynapseBufferEntry
{
    float weigth = 0.0f;
    uint32_t nodeId = UNINIT_STATE_16;
    // total size: 8 Byte
};

//==================================================================================================

struct SynapseBuffer
{
    uint8_t process = 0;
    uint8_t upToDate = 1;
    uint8_t padding[6];

    SynapseBufferEntry buffer[8];

    SynapseBuffer()
    {
        for(uint32_t i = 0; i < 8; i++) {
            buffer[i] = SynapseBufferEntry();
        }
    }

    // total size: 64 Byte
};

//==================================================================================================

struct Synapse
{
    float weight = 0.0;
    uint16_t targetNodeId = UNINIT_STATE_16;
    int8_t sign = 1;
    int8_t multiplicator;
    // total size: 8 Byte
};

//==================================================================================================

struct SynapseSection
{
    uint16_t active = 0;
    uint16_t randomPos = UNINIT_STATE_16;

    uint32_t brickBufferPos = UNINIT_STATE_32;
    uint64_t prev = UNINIT_STATE_64;
    uint64_t next = UNINIT_STATE_64;

    uint32_t hardening = 0;
    uint8_t padding[4];

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
