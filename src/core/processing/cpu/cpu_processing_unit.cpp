/**
 * @file        cpu_processing_unit.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2018 Tobias Anker
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
#include <core/objects/transfer_objects.h>

#include <core/processing/cpu/edge_processing.h>
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

        // process update-messages
        start = std::chrono::system_clock::now();
        const uint32_t numberOfUpdates = updateEdgeSection();
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        KyoukoRoot::monitoringMetaMessage.cpuUpdate = timeValue;
        KyoukoRoot::monitoringMetaMessage.updateTransfers = numberOfUpdates;

        // process axon-messages
        start = std::chrono::system_clock::now();
        const uint32_t numberOfAxons = processEdgeSection();
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        KyoukoRoot::monitoringMetaMessage.cpuProcessing = timeValue;
        KyoukoRoot::monitoringMetaMessage.axonTransfers = numberOfAxons;

        m_phase3->triggerBarrier();
    }
}
