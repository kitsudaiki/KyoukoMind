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

#ifndef GLOBAL_VALUES_HANDLER_H
#define GLOBAL_VALUES_HANDLER_H

#include <common.h>

struct GlobalValues
{
    uint32_t numberOfNodesPerBrick = 1000;

    float sensitivity = 0.0f;
    float lerningValue = 0.0f;
    float gliaValue = 1.0f;
    float initialMemorizing = 0.5f;
    float memorizingOffset = 0.0f;

    float nodeCooldown = 3.0f;
    float actionPotential = 150.0f;
    uint64_t refractionTime = 4;

    float newSynapseBorder = 1.0f;
    float deleteSynapseBorder = 1.0f;

    float outputIndex = 0.0f;

    uint8_t padding[204];
};

#endif // GLOBAL_VALUES_HANDLER_H
