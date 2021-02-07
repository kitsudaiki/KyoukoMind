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

#include <libKitsunemimiPersistence/logger/logger.h>
#include <core/connection_handler/client_connection_handler.h>

InputOutputProcessing::InputOutputProcessing()
{

}

/**
 * @brief InputOutputProcessing::processInputMapping
 */
void
InputOutputProcessing::processInputMapping()
{
    while(KyoukoRoot::m_segment->input_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    float* inputNodes = getBuffer<float>(KyoukoRoot::m_segment->nodeInputBuffer);

    // insert input-values from brick
    for(uint32_t i = 0; i < m_inputMapper.size(); i++)
    {
        InpuMapper mapper = m_inputMapper[i];
        for(uint32_t pos = mapper.start; pos < mapper.end; pos++) {
            inputNodes[pos] = mapper.value;
        }
    }

    KyoukoRoot::m_segment->input_lock.clear(std::memory_order_release);
}

/**
 * @brief InputOutputProcessing::processOutputMapping
 */
void
InputOutputProcessing::processOutputMapping()
{
    // prepare pointer
    float* outputNodes = getBuffer<float>(KyoukoRoot::m_segment->nodeOutputBuffer);

    // insert input-values from brick
    for(uint32_t i = 0; i < m_outputMapper.size(); i++)
    {
        OutputMapper* mapper = &m_outputMapper[i];

        float summedOutput = 0.0f;

        for(uint32_t pos = mapper->start; pos < mapper->end; pos++) {
            summedOutput += outputNodes[pos];
        }

        // make result smooth
        mapper->outBuffer[mapper->outBufferPos] = summedOutput;
        mapper->outBufferPos = (mapper->outBufferPos + 1) % 2;

        float result = 0.0f;
        for(uint32_t x = 0; x < 2; x++) {
            result += mapper->outBuffer[x];
        }
        result /= 2.0f;

        KyoukoRoot::m_clientHandler->sendToClient(std::to_string(result));
        LOG_WARNING("-----------------------------------------------");
        LOG_WARNING("output: " + std::to_string(result));
    }
}

/**
 * @brief InputOutputProcessing::setInput
 * @param input
 */
void
InputOutputProcessing::setInput(const std::string &input)
{
    GlobalValues* globalValues = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    while(KyoukoRoot::m_segment->input_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    const char* inputChar = input.c_str();
    for(uint32_t i = 0; i < input.size(); i++)
    {
        //const float value = (static_cast<float>(inputChar[i]) - 90.0f) * 10.0f;
        if(inputChar[i] == 'a') {
            m_inputMapper[i].value = globalValues->actionPotential;
        } else {
            m_inputMapper[i].value = 0.0f;
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
InputOutputProcessing::registerInput(const uint32_t numberOfInput, const uint32_t inputSize)
{
    while(m_input_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    Brick* inputBrick = KyoukoRoot::m_segment->inputBricks[0];

    for(uint32_t i = 0; i < numberOfInput; i++)
    {
        const uint32_t pos = i * inputSize + inputBrick->nodePos;
        InpuMapper mapper;
        mapper.start = pos;
        mapper.end = pos + inputSize;

        m_inputMapper.push_back(mapper);
    }

    m_input_lock.clear(std::memory_order_release);
}

/**
 * @brief InputOutputProcessing::registerOutput
 * @param numberOfOutputs
 * @param outputSize
 */
void
InputOutputProcessing::registerOutput(const uint32_t numberOfOutputs, const uint32_t outputSize)
{
    while(m_output_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    Brick* outputBrick = KyoukoRoot::m_segment->outputBricks[0];

    for(uint32_t i = 0; i < numberOfOutputs; i++)
    {
        const uint32_t pos = i * outputSize + outputBrick->nodePos;

        OutputMapper mapper;
        mapper.start = pos;
        mapper.end = pos + outputSize;

        m_outputMapper.push_back(mapper);
    }

    m_output_lock.clear(std::memory_order_release);
}

