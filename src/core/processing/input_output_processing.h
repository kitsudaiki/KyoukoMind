/**
 * @file        input_output_processing.h
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

#ifndef INPUT_OUTPUT_PROCESSING_H
#define INPUT_OUTPUT_PROCESSING_H

#include <common.h>

class InputOutputProcessing
{
public:
    InputOutputProcessing();

    void processInputMapping();
    void processOutputMapping();

    void setInput(const std::string& input);

    void registerInput(const uint32_t numberOfInput, const uint32_t inputSize);
    void registerOutput(const uint32_t numberOfOutputs, const uint32_t outputSize);

private:
    struct InpuMapper
    {
        float value = 0.0f;
        uint32_t start = 0;
        uint32_t end = 0;
    };

    struct OutputMapper
    {
        float outBuffer[10];
        uint8_t outBufferPos = 0;

        uint32_t start = 0;
        uint32_t end = 0;
    };

    std::vector<InpuMapper> m_inputMapper;
    std::vector<OutputMapper> m_outputMapper;


    std::vector<float> m_inputs;
    std::atomic_flag m_input_lock = ATOMIC_FLAG_INIT;

    std::vector<float> m_outputs;
    std::atomic_flag m_output_lock = ATOMIC_FLAG_INIT;
};

#endif // INPUT_OUTPUT_PROCESSING_H
