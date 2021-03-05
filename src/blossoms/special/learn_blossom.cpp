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
#include <core/network_manager.h>
#include <core/processing/cpu/output_synapse_processing.h>

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

    KyoukoRoot::m_ioHandler->registerInput(10);
    KyoukoRoot::m_ioHandler->registerOutput(3);

    const std::string input = blossomLeaf.input.get("input")->toValue()->getString();
    const std::string should = blossomLeaf.input.get("should")->toValue()->getString();

    KyoukoRoot::m_ioHandler->setInput(input);
    KyoukoRoot::m_ioHandler->processInputMapping();

    Output* outputs = Kitsunemimi::getBuffer<Output>(KyoukoRoot::m_segment->outputs);
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    for(uint32_t i = 0; i < should.size(); i++) {
        outputs[i].shouldValue = (static_cast<float>(should[i]) - 90.0f) * 10.0f;
    }

    globalValue->doLearn = 1;

    const uint32_t runCount = globalValue->layer + 1;
    for(uint32_t i = 0; i < runCount; i++) {
        KyoukoRoot::m_root->m_networkManager->executeStep();
    }

    float totalDiff = 0.0f;

    do {
        totalDiff = output_learn_step();
    } while(totalDiff >= 0.001f);

    if(totalDiff < 0.001f)
    {
        for(uint32_t i = 0; i < 3; i++) {
            outputs[i].shouldValue = 0.0f;
        }
        KyoukoRoot::m_freezeState = true;
        globalValue->doLearn = 0;
    }
    Kitsunemimi::DataArray outputArray;

    LOG_WARNING("-----------------------------------------------");
    for(uint32_t i = 0; i < KyoukoRoot::m_segment->outputs.numberOfItems; i++)
    {
        outputArray.append(new DataValue(outputs[i].outputValue));
        LOG_WARNING("should" + std::to_string(i) + ": " + std::to_string(outputs[i].shouldValue));
        LOG_WARNING("output" + std::to_string(i) + ": " + std::to_string(outputs[i].outputValue));
    }

    return true;
}
