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
    validationMap.emplace("initial_memorizing", BlossomValidDef(IO_ValueType::INPUT_TYPE, false));
    validationMap.emplace("memorizing_offset", BlossomValidDef(IO_ValueType::INPUT_TYPE, false));
    validationMap.emplace("learning", BlossomValidDef(IO_ValueType::INPUT_TYPE, false));
    validationMap.emplace("sensitivity", BlossomValidDef(IO_ValueType::INPUT_TYPE, false));
    validationMap.emplace("glia_value", BlossomValidDef(IO_ValueType::INPUT_TYPE, false));
}

bool
SetGlobalValues_Blossom::runTask(BlossomLeaf &blossomLeaf,
                                 std::string &)
{
    LOG_DEBUG("update global-values");

    GlobalValues* globalValues = Kitsunemimi::getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    Kitsunemimi::DataMap* input = &blossomLeaf.input;

    if(input->contains("initial_memorizing"))
    {
        DataValue* value = input->get("initial_memorizing")->toValue();
        if(value->isFloatValue()) {
            globalValues->initialMemorizing = value->getFloat();
        } else if(value->isIntValue()) {
            globalValues->initialMemorizing = value->getInt();
        }
    }

    if(input->contains("memorizing_offset"))
    {
        DataValue* value = input->get("memorizing_offset")->toValue();
        if(value->isFloatValue()) {
            globalValues->memorizingOffset = value->getFloat();
        } else if(value->isIntValue()) {
            globalValues->memorizingOffset = value->getInt();
        }
    }

    if(input->contains("sensitivity"))
    {
        DataValue* value = input->get("sensitivity")->toValue();
        if(value->isFloatValue()) {
            globalValues->sensitivity = value->getFloat();
        } else if(value->isIntValue()) {
            globalValues->sensitivity = value->getInt();
        }
    }

    if(input->contains("learning"))
    {
        DataValue* value = input->get("learning")->toValue();
        if(value->isFloatValue()) {
            globalValues->lerningValue = value->getFloat();
        } else if(value->isIntValue()) {
            globalValues->lerningValue = value->getInt();
        }
    }

    if(input->contains("glia_value"))
    {
        DataValue* value = input->get("glia_value")->toValue();
        if(value->isFloatValue()) {
            globalValues->gliaValue = value->getFloat();
        } else if(value->isIntValue()) {
            globalValues->gliaValue = value->getInt();
        }
    }

    return true;
}
