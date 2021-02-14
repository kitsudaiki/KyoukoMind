/**
 * @file        processing_unit_handler.cpp
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

#include <core/processing/processing_unit_handler.h>
#include <core/processing/cpu/cpu_processing_unit.h>
#include <core/processing/gpu/gpu_processing_uint.h>

#include <libKitsunemimiCommon/threading/thread.h>
#include <libKitsunemimiCommon/threading/barrier.h>

#include <libKitsunemimiOpencl/gpu_handler.h>
#include <libKitsunemimiOpencl/gpu_interface.h>

#include <core/objects/segment.h>

#include <kyouko_root.h>

/**
 * @brief ProcessingUnitHandler::ProcessingUnitHandler
 * @param brickHandler
 */
ProcessingUnitHandler::ProcessingUnitHandler()
{
}

/**
 * @brief ProcessingUnitHandler::~ProcessingUnitHandler
 */
ProcessingUnitHandler::~ProcessingUnitHandler()
{
    closeAllProcessingUnits();
}

/**
 * @brief ProcessingUnitHandler::initProcessingUnits
 * @param numberOfThreads
 * @return
 */
bool
ProcessingUnitHandler::initProcessingUnits(Kitsunemimi::Barrier* phase1,
                                           Kitsunemimi::Barrier* phase2,
                                           Kitsunemimi::Barrier* phase3,
                                           const uint16_t numberOfThreads)
{
    // init cpu
    for(uint16_t i = 0; i < numberOfThreads; i++)
    {
        CpuProcessingUnit* newUnit = new CpuProcessingUnit();
        m_cpuProcessingUnits.push_back(newUnit);

        newUnit->m_phase1 = phase1;
        newUnit->m_phase2 = phase2;
        newUnit->m_phase3 = phase3;

        newUnit->startThread();
    }

    // init gpu
    /*m_gpuHandler = new Kitsunemimi::Opencl::GpuHandler();
    for(uint16_t i = 0; i < m_gpuHandler->m_interfaces.size(); i++)
    {
        GpuProcessingUnit* newUnit = new GpuProcessingUnit(m_gpuHandler->m_interfaces.at(i));
        m_gpuProcessingUnits.push_back(newUnit);

        // init gpu
        Segment* segment = KyoukoRoot::m_segment;
        const uint32_t numberOfBricks = static_cast<uint32_t>(segment->bricks.itemCapacity);
        if(newUnit->initializeGpu(*segment, numberOfBricks) == false) {
            return false;
        }

        newUnit->m_phase1 = phase1;
        newUnit->m_phase2 = phase2;
        newUnit->m_phase3 = phase3;

        newUnit->startThread();
    }*/

    return true;
}

/**
 * @brief ProcessingUnitHandler::closeAllProcessingUnits close all processing-units, if exist
 * @return false, if no processing-units exist, else true
 */
bool
ProcessingUnitHandler::closeAllProcessingUnits()
{
    if(m_cpuProcessingUnits.size() == 0) {
        return false;
    }

    for(uint32_t i = 0; i < m_cpuProcessingUnits.size(); i++)
    {
        CpuProcessingUnit* unit = m_cpuProcessingUnits.at(i);
        delete unit;
    }

    m_cpuProcessingUnits.clear();

    return true;
}
