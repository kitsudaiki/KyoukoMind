/**
 * @file        register_output_blossom.cpp
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

#include "register_output_blossom.h"

#include <libKitsunemimiPersistence/logger/logger.h>
#include <core/objects/segment.h>
#include <core/processing/input_output_processing.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

RegisterOutputBlossom::RegisterOutputBlossom()
    : Kitsunemimi::Sakura::Blossom()
{
    validationMap.emplace("size", BlossomValidDef(IO_ValueType::INPUT_TYPE, true));
}

bool
RegisterOutputBlossom::runTask(BlossomLeaf &blossomLeaf,
                               std::string &errorMessage)
{
    LOG_DEBUG("register output");

    const int size = blossomLeaf.input.get("size")->toValue()->getInt();

    if(size < 0)
    {
        errorMessage = "number of outputs to register must be a positive value";
        return false;
    }

    KyoukoRoot::m_ioHandler->registerOutput(static_cast<uint32_t>(size));

    return true;
}

