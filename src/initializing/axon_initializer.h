/**
 * @file        axon_initializer.h
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

#ifndef AXON_INITIALIZER_H
#define AXON_INITIALIZER_H

#include <common.h>
#include "init_meta_data.h"
#include <core/objects/brick.h>

class NextChooser;

struct Neighbor;
class Segment;

struct NewAxon {
    uint32_t x = 0;
    uint32_t y = 0;
};


bool createAxons(Segment &segment,
                 const std::vector<std::vector<InitMetaDataEntry> > &networkMetaStructure);

NewAxon getNextAxonPathStep(const uint32_t x,
                            const uint32_t y,
                            const uint8_t inputSide,
                            uint32_t &currentStep,
                            const std::vector<std::vector<InitMetaDataEntry>> &networkMetaStructure);

uint8_t chooseNextSide(const uint8_t initialSide,
                       uint32_t* neighbors);

#endif // AXON_INITIALIZER_H
