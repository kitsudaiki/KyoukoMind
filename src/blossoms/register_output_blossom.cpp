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

#include "register_output_blossom.h"

#include <libKitsunemimiPersistence/logger/logger.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

RegisterOutputBlossom::RegisterOutputBlossom()
    : Kitsunemimi::Sakura::Blossom()
{
    validationMap.emplace("id", BlossomValidDef(IO_ValueType::INPUT_TYPE, true));
    validationMap.emplace("position", BlossomValidDef(IO_ValueType::INPUT_TYPE, true));
    validationMap.emplace("range", BlossomValidDef(IO_ValueType::INPUT_TYPE, true));
}

bool
RegisterOutputBlossom::runTask(BlossomLeaf &blossomLeaf,
                               std::string &errorMessage)
{
    LOG_DEBUG("register output");

    Kitsunemimi::DataMap* input = &blossomLeaf.input;
    const uint32_t id = static_cast<uint32_t>(input->get("id")->toValue()->getInt());
    const uint32_t position = static_cast<uint32_t>(input->get("position")->toValue()->getInt());
    const uint32_t range = static_cast<uint32_t>(input->get("range")->toValue()->getInt());

    KyoukoRoot::registeredOutputs.insert(std::make_pair(id, arrayPos(position, range)));

    return true;
}

