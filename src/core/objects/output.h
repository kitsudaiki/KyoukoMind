/**
 * @file        output.h
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

#ifndef OUTPUT_ITEM_H
#define OUTPUT_ITEM_H

#include <common.h>
#include <libKitsunemimiCommon/buffer/item_buffer.h>

//==================================================================================================

struct OutputSynapse
{
    float border = 0.0;
    float weight = 0.0;
    uint32_t targetId = UNINIT_STATE_32;
    uint8_t newOne = 0;
    uint8_t active = 0;
    uint8_t padding[2];
    // total size: 16 Byte
};

//==================================================================================================

struct OutputSynapseSection
{
    uint16_t status = Kitsunemimi::ItemBuffer::ACTIVE_SECTION;
    uint16_t randomPos = UNINIT_STATE_16;

    uint64_t prev = UNINIT_STATE_64;

    uint32_t hardening = 0;

    uint32_t newOnes = 0;
    uint32_t total = 0;

    float diffNew = 0.0f;
    float diffTotal = 0.0f;

    OutputSynapse synapses[OUTPUT_SYNAPSES_PER_SECTION];


    OutputSynapseSection()
    {
        for(uint32_t i = 0; i < OUTPUT_SYNAPSES_PER_SECTION; i++) {
            synapses[i] = OutputSynapse();
        }
    }

    // total size: 4096 Byte
};

//==================================================================================================

struct Output
{
    float outputValue = 0.0f;
    float shouldValue = 0.0f;
};

//==================================================================================================

#endif // OUTPUT_ITEM_H
