/**
 * @file        input_output_processing.cpp
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

#include "input_output_processing.h"

#include <core/objects/segment.h>
#include <core/objects/node.h>
#include <core/objects/global_values.h>
#include <core/objects/output.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <core/connection_handler/client_connection_handler.h>

InputOutputProcessing::InputOutputProcessing() {}

/**
 * @brief InputOutputProcessing::processInputMapping
 */
void
InputOutputProcessing::processInputMapping()
{
    while(KyoukoRoot::m_segment->input_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    Brick* inputBrick = KyoukoRoot::m_segment->inputBricks[0];

    // insert input-values from brick
    float* inputNodes = Kitsunemimi::getBuffer<float>(KyoukoRoot::m_segment->nodeInputBuffer);
    for(uint32_t i = 0; i < m_inputMapper.size(); i++) {
        inputNodes[i + inputBrick->nodePos] = m_inputMapper[i];
    }

    KyoukoRoot::m_segment->input_lock.clear(std::memory_order_release);
}

/**
 * @brief InputOutputProcessing::processOutputMapping
 */
void
InputOutputProcessing::processOutputMapping()
{
    Output* outputs = Kitsunemimi::getBuffer<Output>(KyoukoRoot::m_segment->outputs);
    Kitsunemimi::DataArray outputArray;

    LOG_WARNING("-----------------------------------------------");
    for(uint32_t i = 0; i < KyoukoRoot::m_segment->outputs.numberOfItems; i++)
    {
        outputArray.append(new DataValue(outputs[i].outputValue));
        LOG_WARNING("should" + std::to_string(i) + ": " + std::to_string(outputs[i].shouldValue));
        LOG_WARNING("output" + std::to_string(i) + ": " + std::to_string(outputs[i].outputValue));
    }

    KyoukoRoot::m_clientHandler->sendToClient(outputArray.toString());
}

/**
 * @brief InputOutputProcessing::setInput
 * @param input
 */
void
InputOutputProcessing::setInput(const std::string &input)
{
    while(KyoukoRoot::m_segment->input_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    const char* inputChar = input.c_str();
    for(uint32_t i = 0; i < input.size(); i++)
    {
        for(uint32_t j = 0; j < 10; j++)
        {
            const uint32_t pos = j + i * 10;
            if(inputChar[i] == '0') {
                m_inputMapper[pos] = 0.0f;
            } else {
                m_inputMapper[pos] = (static_cast<float>(inputChar[i])) * 10.0f;
            }
        }
    }

    KyoukoRoot::m_segment->input_lock.clear(std::memory_order_release);
}

/**
 * @brief InputOutputProcessing::registerInput
 * @param numberOfInput
 * @param inputSize
 */
void
InputOutputProcessing::registerInput(const uint32_t numberOfInput)
{
    while(KyoukoRoot::m_segment->input_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    for(uint32_t i = 0; i < numberOfInput * 10; i++) {
        m_inputMapper.push_back(0.0f);
    }

    KyoukoRoot::m_segment->input_lock.clear(std::memory_order_release);
}

/**
 * @brief InputOutputProcessing::registerOutput
 * @param numberOfOutputs
 * @param outputSize
 */
void
InputOutputProcessing::registerOutput(const uint32_t numberOfOutputs)
{
    Brick* outputBrick = KyoukoRoot::m_segment->outputBricks[0];

}

