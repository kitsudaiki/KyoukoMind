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
    validationMap.emplace("result", BlossomValidDef(IO_ValueType::OUTPUT_TYPE, true));
}

bool
LearnBlossom::runTask(BlossomLeaf &blossomLeaf,
                      std::string &errorMessage)
{
    LOG_DEBUG("start learning");

    const std::string input = blossomLeaf.input.get("input")->toValue()->getString();
    const std::string should = blossomLeaf.input.get("should")->toValue()->getString();

    KyoukoRoot::m_ioHandler->setInput(input);
    KyoukoRoot::m_ioHandler->processInputMapping();

    Output* outputs = Kitsunemimi::getBuffer<Output>(KyoukoRoot::m_segment->outputs);
    const uint64_t numberOfOutputs = KyoukoRoot::m_segment->outputs.numberOfItems;
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    for(uint32_t i = 0; i < should.size(); i++) {
        outputs[i].shouldValue = (static_cast<float>(should[i]) - 90.0f) * 10.0f;
    }

    globalValue->doLearn = 1;

    // learn until output-section
    const uint32_t runCount = globalValue->layer + 2;
    for(uint32_t i = 0; i < runCount; i++) {
        KyoukoRoot::m_root->m_networkManager->executeStep();
    }

    // learn current state
    uint32_t timeout = 1000;
    float totalDiff = 0.0f;
    do
    {
        totalDiff = output_learn_step();
        timeout--;
    }
    while(totalDiff >= 0.001f
          && timeout > 0);

    bool result = false;

    // if desired state was reached, than freeze lerned state
    if(totalDiff < 0.001f)
    {
        result = true;
        KyoukoRoot::m_freezeState = true;
        globalValue->lerningValue = 100000.0f;
        KyoukoRoot::m_root->m_networkManager->executeStep();
        output_node_processing();
        globalValue->doLearn = 0;
        globalValue->lerningValue = 0.0f;
        KyoukoRoot::m_freezeState = false;
    }

    // resett desired should-output
    for(uint64_t i = 0; i < numberOfOutputs; i++) {
        outputs[i].shouldValue = 0.0f;
    }

    blossomLeaf.output.insert("result", new DataValue(result));
    Kitsunemimi::DataArray outputArray;

    return true;
}
