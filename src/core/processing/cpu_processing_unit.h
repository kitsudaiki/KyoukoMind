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

#ifndef KYOUKOMIND_CPU_PROCESSING_UNIT_H
#define KYOUKOMIND_CPU_PROCESSING_UNIT_H

#include <common.h>
#include <libKitsunemimiCommon/threading/thread.h>

class AbstractSegment;

class CpuProcessingUnit
        : public Kitsunemimi::Thread
{
public:
    CpuProcessingUnit(const std::string &threadName);
    ~CpuProcessingUnit();

protected:
    void run();

    uint64_t reductionCounter = 0;

    void learnSegmentForward(AbstractSegment* segment);
    void processSegment(AbstractSegment* segment);
};

#endif // KYOUKOMIND_CPU_PROCESSING_UNIT_H
