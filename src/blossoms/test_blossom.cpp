/**
 * @file        test_blossom.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2020 Tobias Anker
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

#include "test_blossom.h"

#include <libKitsunemimiPersistence/logger/logger.h>

using namespace Kitsunemimi::Sakura;

TestBlossom::TestBlossom()
    : Kitsunemimi::Sakura::Blossom()
{
    validationMap.emplace("input", BlossomValidDef(IO_ValueType::INPUT_TYPE, true));
    validationMap.emplace("output", BlossomValidDef(IO_ValueType::OUTPUT_TYPE, true));
}

bool
TestBlossom::runTask(BlossomLeaf &blossomLeaf, std::string &)
{
    LOG_DEBUG("TestBlossom");
    Kitsunemimi::DataValue* value = blossomLeaf.input.get("input")->toValue();
    blossomLeaf.output.insert("output", new Kitsunemimi::DataValue(42));
    return true;
}

