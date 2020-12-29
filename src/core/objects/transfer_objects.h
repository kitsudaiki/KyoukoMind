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

#ifndef TRANSFER_OBJECTS_H
#define TRANSFER_OBJECTS_H

#include <common.h>

//==================================================================================================

struct SynapseTransfer
{
    uint32_t brickId = UNINIT_STATE_32;
    uint32_t nodeBrickId = UNINIT_STATE_32;
    uint32_t synapseSectionId = UNINIT_STATE_32;
    uint32_t sourceEdgeId = UNINIT_STATE_32;
    uint16_t positionInEdge = UNINIT_STATE_16;
    uint8_t isNew = 0;
    uint8_t padding[1];
    float weight = 0.0f;
    // total size: 24 Byte
};

//==================================================================================================

struct AxonTransfer
{
    float weight = 0.0f;
    uint32_t brickId = UNINIT_STATE_32;
    // total size: 8 Byte
};

//==================================================================================================

struct UpdateTransfer
{
    uint32_t targetId = UNINIT_STATE_32;
    uint8_t positionInEdge = UNINIT_STATE_8;
    uint8_t deleteEdge = 0;
    uint8_t padding[2];
    float newWeight = 0.0f;
    // total size: 12 Byte
};


//==================================================================================================

#endif // TRANSFER_OBJECTS_H
