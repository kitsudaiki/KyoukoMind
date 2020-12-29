/**
 * @file        kyouko_root.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2018 Tobias Anker
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

#ifndef NODES_H
#define NODES_H

#include <common.h>

//==================================================================================================

struct Node
{
    float currentState = 0.0f;
    float border = 100.0f;
    float potential = 0.0f;

    uint8_t refractionTime = 0;
    uint8_t active = 0;
    uint8_t padding[2];

    uint32_t brickId = 0;
    uint32_t targetBrickDistance = 0;

    // total size: 24 Byte
};

//==================================================================================================

#endif // NODES_H
