/**
 * @file        node.h
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

#ifndef KYOUKOMIND_NODES_H
#define KYOUKOMIND_NODES_H

#include <common.h>

struct InputNode
{
    float weight = 0.0f;
    uint32_t targetBorderId = 0;

    // total size: 8 Byte
};

struct OutputNode
{
    float outputWeight = 0.0f;
    float shouldValue = 0.0f;
    uint32_t targetBorderId = 0;
    uint8_t padding[4];

    // total size: 16 Byte
};

//==================================================================================================

struct Node
{
    float input = 0.0f;
    float border = 100.0f;
    float potential = 0.0f;

    float delta = 0.0f;

    uint8_t refractionTime = 1;
    uint8_t active = 0;
    uint8_t upToDate = 0;
    bool isInit = false;

    uint32_t brickId = 0;

    uint32_t targetBorderId = UNINIT_STATE_32;
    uint32_t targetSectionId = UNINIT_STATE_32;

    // total size: 32 Byte
};

//==================================================================================================

#endif // KYOUKOMIND_NODES_H
