/**
 * @file        cpu_processing_unit.h
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

#ifndef PROCESSING_UNIT_H
#define PROCESSING_UNIT_H

#include <common.h>
#include <libKitsunemimiCommon/threading/thread.h>

namespace Kitsunemimi {
class Barrier;
}

class GlobalValuesHandler;
class Segment;
struct Brick;

class CpuProcessingUnit
        : public Kitsunemimi::Thread
{

public:
    CpuProcessingUnit();

    void run();

    Kitsunemimi::Barrier* m_phase2 = nullptr;
    Kitsunemimi::Barrier* m_phase3 = nullptr;

private:
    DataBuffer m_clientBuffer;
    DataBuffer m_monitoringBuffer;
};

#endif // PROCESSING_UNIT_H
