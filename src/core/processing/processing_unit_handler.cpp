/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <core/processing/processing_unit_handler.h>
#include <core/processing/cpu/cpu_processing_unit.h>
#include <core/processing/gpu/gpu_processing_uint.h>

#include <libKitsunemimiCommon/threading/thread.h>
#include <libKitsunemimiCommon/threading/barrier.h>

#include <libKitsunemimiOpencl/gpu_handler.h>
#include <libKitsunemimiOpencl/gpu_interface.h>

#include <core/object_handling/segment.h>

#include <kyouko_root.h>

namespace KyoukoMind
{

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
ProcessingUnitHandler::initProcessingUnits(Kitsunemimi::Barrier* cpuBarrier,
                                           Kitsunemimi::Barrier* gpuBarrier,
                                           const uint16_t numberOfThreads)
{
    // init cpu
    for(uint16_t i = 0; i < numberOfThreads; i++)
    {
        CpuProcessingUnit* newUnit = new CpuProcessingUnit();
        m_cpuProcessingUnits.push_back(newUnit);
        newUnit->m_barrier = cpuBarrier;
        newUnit->startThread();
    }

    // init gpu
    m_gpuHandler = new Kitsunemimi::Opencl::GpuHandler();
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
        newUnit->m_gpuBarrier = gpuBarrier;
        newUnit->m_cpuBarrier = cpuBarrier;

        newUnit->startThread();
    }

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
        unit->stopThread();
        delete unit;
    }

    m_cpuProcessingUnits.clear();

    return true;
}

} // namespace KyoukoMind
