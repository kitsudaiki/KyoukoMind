/**
 * @file        set_global_values_blossom.cpp
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

#include "set_global_values_blossom.h"

#include <libKitsunemimiPersistence/logger/logger.h>

#include <core/objects/segment.h>
#include <kyouko_root.h>
#include <core/objects/global_values.h>

using namespace Kitsunemimi::Sakura;

SetGlobalValues_Blossom::SetGlobalValues_Blossom()
{
    validationMap.emplace("memorizing", BlossomValidDef(IO_ValueType::INPUT_TYPE, false));
    validationMap.emplace("learning", BlossomValidDef(IO_ValueType::INPUT_TYPE, false));
    validationMap.emplace("glia_value", BlossomValidDef(IO_ValueType::INPUT_TYPE, false));
}

bool
SetGlobalValues_Blossom::runTask(BlossomLeaf &blossomLeaf,
                                 std::string &)
{
    LOG_DEBUG("update global-values");

    return true;
}
