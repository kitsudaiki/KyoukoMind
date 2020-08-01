/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <core/processing/cpu/cpu_processing_unit.h>
#include <kyouko_root.h>

#include <core/obj_converter.h>

#include <core/object_handling/segment.h>
#include <core/object_handling/global_values.h>
#include <core/object_handling/brick.h>

#include <core/processing/objects/transfer_objects.h>

#include <core/processing/cpu/edge_processing.h>
#include <core/processing/gpu/gpu_processing_uint.h>

#include <libKitsunemimiPersistence/logger/logger.h>

#include <libKitsunemimiCommon/threading/barrier.h>

namespace KyoukoMind
{

/**
 * @brief ProcessingUnit::ProcessingUnit
 * @param brickHandler
 */
CpuProcessingUnit::CpuProcessingUnit()
{
}

/**
 * @brief ProcessingUnit::run
 */
void
CpuProcessingUnit::run()
{
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;

    while(!m_abort)
    {
        m_phase1->triggerBarrier();
        m_phase2->triggerBarrier();

        start = std::chrono::system_clock::now();
        std::cout<<"number of synapse-sections1: "
                 <<(KyoukoRoot::m_segment->synapses.numberOfItems)
                 <<std::endl;

        const uint32_t numberOfActiveUpdates = updateEdgeSection();
        const uint32_t numberOfActiveAxons = processEdgeSection();

        std::cout<<"number of update-transfers: "
                 <<numberOfActiveUpdates
                 <<std::endl;
        std::cout<<"number of active Axons: "
                 <<numberOfActiveAxons
                 <<std::endl;
        std::cout<<"number of synapse-sections2: "
                 <<(KyoukoRoot::m_segment->synapses.numberOfItems)
                 <<std::endl;

        end = std::chrono::system_clock::now();
        const float duration = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        LOG_DEBUG("time: " + std::to_string(duration / 1000.0f) + '\n');

        m_phase3->triggerBarrier();
    }
}

} // namespace KyoukoMind
