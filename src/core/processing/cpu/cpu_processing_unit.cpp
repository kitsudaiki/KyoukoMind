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

/**
 * @brief ProcessingUnit::ProcessingUnit
 * @param brickHandler
 */
CpuProcessingUnit::CpuProcessingUnit() {}

/**
 * @brief ProcessingUnit::run
 */
void
CpuProcessingUnit::run()
{
    while(m_abort == false)
    {
        m_phase1->triggerBarrier();
        m_phase2->triggerBarrier();

        KyoukoRoot::monitoringMetaMessage.numberOfUpdateMessages = updateEdgeSection();
        KyoukoRoot::monitoringMetaMessage.numberOfAxonMessages = processEdgeSection();

        m_phase3->triggerBarrier();
    }
}
