/**
 * @file        cpu_processing_unit.cpp
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

#include <core/processing/cpu/cpu_processing_unit.h>
#include <kyouko_root.h>

#include <import_export/obj_converter.h>

#include <libKitsunemimiCommon/threading/barrier.h>

#include <core/objects/segment.h>

#include <core/objects/global_values.h>
#include <core/objects/brick.h>

#include <core/processing/cpu/synapse_processing.h>
#include <core/processing/cpu/output_synapse_processing.h>
#include <core/processing/gpu/gpu_processing_uint.h>

#include <libKitsunemimiPersistence/logger/logger.h>

/**
 * @brief constructor
 */
CpuProcessingUnit::CpuProcessingUnit() {}

/**
 * @brief run cpu-unit
 */
void
CpuProcessingUnit::run()
{
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    float timeValue = 0.0f;

    while(m_abort == false)
    {
        m_phase1->triggerBarrier();

        m_phase2->triggerBarrier();

        start = std::chrono::system_clock::now();
        KyoukoRoot::m_segment->outputValue = 0.0f;
        node_processing();
        output_node_processing();
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        KyoukoRoot::monitoringMetaMessage.gpuNode = timeValue;

        m_phase3->triggerBarrier();
    }
}
