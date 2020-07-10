/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <core/processing/processing_unit.h>
#include <kyouko_root.h>

#include <core/object_handling/network_segment.h>
#include <core/processing/objects/transfer_objects.h>
#include <core/global_values.h>

#include <core/object_handling/brick.h>
#include <core/processing/objects/container_definitions.h>

#include <core/processing/external/message_processing.h>
#include <core/processing/gpu_interface.h>

#include <libKitsunemimiPersistence/logger/logger.h>

namespace KyoukoMind
{

/**
 * @brief ProcessingUnit::ProcessingUnit
 * @param brickHandler
 */
ProcessingUnit::ProcessingUnit()
{
    m_block = true;
}

/**
 * @brief ProcessingUnit::run
 */
void
ProcessingUnit::run()
{
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;

    NetworkSegment* segment = KyoukoRoot::m_segment;

    while(!m_abort)
    {
        if(m_block) {
            blockThread();
        }

        end = std::chrono::system_clock::now();
        const float duration = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        LOG_DEBUG("time: " + std::to_string(duration / 1000.0f) + '\n');

        if(USE_GPU)
        {
            // copy transfer-edges to gpu
            start = std::chrono::system_clock::now();
            KyoukoRoot::m_gpuInterface->copyEdgesToGpu(*segment);
            end = std::chrono::system_clock::now();
            const float gpu0 = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
            LOG_DEBUG("time copy to gpu: " + std::to_string(gpu0 / 1000.0f) + '\n');

            // run process on gpu
            start = std::chrono::system_clock::now();
            KyoukoRoot::m_gpuInterface->runOnGpu();
            end = std::chrono::system_clock::now();
            const float gpu1 = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
            LOG_DEBUG("gpu run-time: " + std::to_string(gpu1 / 1000.0f) + '\n');

            // copy result from gpu to host
            start = std::chrono::system_clock::now();
            KyoukoRoot::m_gpuInterface->copyAxonsFromGpu();
            end = std::chrono::system_clock::now();
            const float gpu2 = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
            LOG_DEBUG("time copy from gpu: " + std::to_string(gpu2 / 1000.0f) + '\n');
        }

        segment->synapseEdges.resetBufferContent();

        // block thread until next cycle if queue is empty
        blockThread();

        start = std::chrono::system_clock::now();


        uint32_t count = 0;
        AxonTransfer* axons = static_cast<AxonTransfer*>(segment->axonEdges.buffer.data);
        for(uint32_t i = 0; i < segment->axonEdges.numberOfItems; i++)
        {

        }
        std::cout<<"number of active Axons: "<<count<<std::endl;
    }
}

} // namespace KyoukoMind
