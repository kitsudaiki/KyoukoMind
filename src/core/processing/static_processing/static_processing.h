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

#ifndef STATIC_PROCESSING_H
#define STATIC_PROCESSING_H

#include <common.h>
#include <libKitsunemimiCommon/threading/thread.h>

struct Output;

namespace Kitsunemimi {
namespace Opencl {
class GpuHandler;
}
}
class GpuProcessingUnit;

class StaticProcessing
{
public:
    StaticProcessing();
    virtual ~StaticProcessing();

    bool learn();
    bool execute();
private:
    bool learnStep();
    virtual void executeStep(const uint32_t runs) = 0;
    virtual void reductionLearning(const uint32_t runs) = 0;
    virtual void updateLearning(const uint32_t runs) = 0;
};

#endif // STATIC_PROCESSING_H
