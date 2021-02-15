/**
 * @file        set_input_blossom.cpp
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

#include "set_input_blossom.h"

#include <libKitsunemimiPersistence/logger/logger.h>
#include <kyouko_root.h>
#include <core/processing/input_output_processing.h>

using namespace Kitsunemimi::Sakura;

SetInputBlossom::SetInputBlossom()
    : Kitsunemimi::Sakura::Blossom()
{
    validationMap.emplace("input", BlossomValidDef(IO_ValueType::INPUT_TYPE, true));
}

bool
SetInputBlossom::runTask(BlossomLeaf &blossomLeaf,
                         std::string &errorMessage)
{
    LOG_DEBUG("set input");

    const std::string input = blossomLeaf.input.get("input")->toValue()->getString();

    KyoukoRoot::m_ioHandler->setInput(input);

    return true;
}
