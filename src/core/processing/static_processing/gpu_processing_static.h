/**
 * @file        gpu_processing_static.h
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

#ifndef GPUPROCESSINGSTATIC_H
#define GPUPROCESSINGSTATIC_H

#include <core/processing/static_processing/static_processing.h>

class GpuProcessingStatic
        : public StaticProcessing
{
public:
    GpuProcessingStatic();

private:
    Kitsunemimi::Opencl::GpuHandler* m_gpuHandler = nullptr;
    GpuProcessingUnit* m_gpu = nullptr;

    void executeStep(const uint32_t runs);
    void reductionLearning(const uint32_t runs);
    void updateLearning(const uint32_t runs);
};

#endif // GPUPROCESSINGSTATIC_H
