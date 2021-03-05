/**
 * @file        learn_blossom.cpp
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

#include "learn_blossom.h"

#include <kyouko_root.h>
#include <core/processing/input_output_processing.h>

#include <core/objects/global_values.h>
#include <core/objects/output.h>
#include <core/objects/segment.h>

#include <libKitsunemimiPersistence/logger/logger.h>

using namespace Kitsunemimi::Sakura;

LearnBlossom::LearnBlossom()
    : Kitsunemimi::Sakura::Blossom()
{
    validationMap.emplace("input", BlossomValidDef(IO_ValueType::INPUT_TYPE, true));
    validationMap.emplace("should", BlossomValidDef(IO_ValueType::INPUT_TYPE, true));
}

bool
LearnBlossom::runTask(BlossomLeaf &blossomLeaf,
                      std::string &errorMessage)
{
    LOG_DEBUG("start learning");

    const std::string input = blossomLeaf.input.get("input")->toValue()->getString();
    const std::string should = blossomLeaf.input.get("should")->toValue()->getString();

    KyoukoRoot::m_ioHandler->setInput(input);

    Output* outputs = Kitsunemimi::getBuffer<Output>(KyoukoRoot::m_segment->outputs);
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    for(uint32_t i = 0; i < should.size(); i++) {
        outputs[i].shouldValue = (static_cast<float>(should[i]) - 90.0f) * 10.0f;
    }

    globalValue->doLearn = 1;

    return true;
}
