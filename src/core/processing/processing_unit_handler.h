/**
 * @file        processing_unit_handler.h
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

#ifndef PROCESSING_UNIT_HANDLER_H
#define PROCESSING_UNIT_HANDLER_H

#include <common.h>

namespace Kitsunemimi {
class Barrier;
namespace Opencl {
class GpuHandler;
}
}

class GpuProcessingUnit;

class ProcessingUnitHandler
{
public:
    ProcessingUnitHandler();
    ~ProcessingUnitHandler();

    void shareNewTask(const ThreadTask newTask);

    std::vector<GpuProcessingUnit*> m_gpuProcessingUnits;

    bool initProcessingUnits(Kitsunemimi::Barrier* startBarrier,
                             Kitsunemimi::Barrier* endBarrier,
                             const uint16_t numberOfThreads);
    bool closeAllProcessingUnits();

private:
    Kitsunemimi::Opencl::GpuHandler* m_gpuHandler = nullptr;
};

#endif // PROCESSING_UNIT_HANDLER_H
