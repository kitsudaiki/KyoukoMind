/**
 * @file        static_processing.h
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

#ifndef LERNER_H
#define LERNER_H

#include <common.h>
#include <libKitsunemimiCommon/threading/thread.h>

namespace Kitsunemimi {
class Barrier;
}

class ProcessingUnitHandler;

struct OutputSegmentMeta;
struct Output;

struct Batch {
    float buffer[2400];
    uint32_t counter = 0;
};

namespace Kitsunemimi {
namespace Opencl {
class GpuHandler;
}
}
class GpuProcessingUnit;

class StaticProcessing
{
public:
    StaticProcessing(const bool useGpu);

    bool learn();
    bool execute();

    Batch batchs[10];

    float buffer[2400];
private:
    Kitsunemimi::Opencl::GpuHandler* m_gpuHandler = nullptr;
    GpuProcessingUnit* m_gpu = nullptr;
    bool m_useGpu = false;

    uint32_t checkOutput(OutputSegmentMeta *segmentMeta, Output *outputs);

    bool learnStep();
    void executeStep(const uint32_t runs);

    bool learnPhase1();
    bool learnPhase2();

    ProcessingUnitHandler* m_processingUnitHandler = nullptr;
    Kitsunemimi::Barrier* m_startBarrier = nullptr;
    Kitsunemimi::Barrier* m_endBarrier = nullptr;
};

#endif // LERNER_H
