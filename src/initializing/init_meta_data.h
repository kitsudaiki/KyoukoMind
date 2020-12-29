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

#ifndef INIT_META_DATA_H
#define INIT_META_DATA_H

#include <common.h>

class Brick;

enum BrickTypes
{
    UNDEFINED_BRICK = 0,
    EMPTY_BRICK = 1,
    EDGE_BRICK = 2,
    NODE_BRICK = 3,
    OUTPUT_NODE_BRICK = 4,
};


/**
 * @brief The InitMetaDataEntry struct
 */
struct InitMetaDataEntry
{
    uint8_t type = EMPTY_BRICK;
    Brick* brick = nullptr;
};

#endif // INIT_META_DATA_H
