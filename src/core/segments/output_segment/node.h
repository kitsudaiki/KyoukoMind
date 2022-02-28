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

#ifndef KYOUKOMIND_OUTPUT_NODES_H
#define KYOUKOMIND_OUTPUT_NODES_H

#include <common.h>

struct OutputNode
{
    float outputWeight = 0.0f;
    float shouldValue = 0.0f;
    uint32_t targetBorderId = 0;
    uint8_t padding[4];

    // total size: 16 Byte
};

//==================================================================================================

#endif // KYOUKOMIND_OUTPUT_NODES_H
