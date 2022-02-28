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

#ifndef KYOUKOMIND_STATIC_NODES_H
#define KYOUKOMIND_STATIC_NODES_H

#include <common.h>

//==================================================================================================

struct StaticNode
{
    float value = 0.0f;
    float border = 0.0f;
    float potential = 0.0f;

    float delta = 0.0f;

    uint32_t brickId = 0;
    uint32_t numberOfConnections = 0;
    uint32_t targetConnectionPos = UNINIT_STATE_32;

    uint32_t targetBorderId = UNINIT_STATE_32;

    // total size: 32 Byte
};

//==================================================================================================

#endif // KYOUKOMIND_STATIC_NODES_H
