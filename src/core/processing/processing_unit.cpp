/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <core/processing/processing_unit.h>
#include <kyouko_root.h>

#include <core/object_handling/segment.h>
#include <core/processing/objects/transfer_objects.h>
#include <core/global_values.h>

#include <core/object_handling/brick.h>
#include <core/processing/objects/container_definitions.h>

#include <core/processing/external/message_processing.h>
#include <core/processing/internal/edge_processing.h>
#include <core/processing/gpu_interface.h>

#include <libKitsunemimiPersistence/logger/logger.h>

#include <core/obj_converter.h>
#include <libKitsunemimiPersistence/files/text_file.h>

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

    Segment* segment = KyoukoRoot::m_segment;

    while(!m_abort)
    {
        if(m_block) {
            blockThread();
        }

        if(USE_GPU)
        {
            // copy transfer-edges to gpu
            start = std::chrono::system_clock::now();
            KyoukoRoot::m_gpuInterface->copySynapseTransfersToGpu(*segment);
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
            KyoukoRoot::m_gpuInterface->copyAxonTransfersFromGpu();
            end = std::chrono::system_clock::now();
            const float gpu2 = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
            LOG_DEBUG("time copy from gpu: " + std::to_string(gpu2 / 1000.0f) + '\n');
        }

        segment->synapseTransfers.deleteAll();

        // block thread until next cycle if queue is empty
        blockThread();

        start = std::chrono::system_clock::now();

        EdgeSection* edges = getBuffer<EdgeSection>(segment->edges);

        uint32_t count = 0;

        // process update-transfers
        UpdateTransfer* updates = getBuffer<UpdateTransfer>(segment->updateTransfers);
        for(uint32_t i = 0; i < segment->updateTransfers.itemCapacity; i++)
        {
            if(updates[i].weightDiff == 0.0f) {
                continue;
            }

            count++;
            const UpdateTransfer container = updates[i];
            updateEdgeSection(edges[container.targetId],
                              container.positionInEdge,
                              container.weightDiff,
                              container.deleteEdge);
        }
        std::cout<<"number of update-transfers: "<<count<<std::endl;

        count = 0;

        // process axon-transfers
        AxonTransfer* axons = getBuffer<AxonTransfer>(segment->axonTransfers);
        // test-input
        for(uint32_t i = 0; i < 1; i++)
        {
            axons[i].weight = 100.0f;
        }
        for(uint32_t i = 0; i < segment->axonTransfers.itemCapacity; i++)
        {
            if(axons[i].weight == 0.0f) {
                continue;
            }
            count++;
            processEdgeSection(edges[i],
                               axons[i].weight,
                               i,
                               edges[i].targetBrickId);
        }

        std::cout<<"number of active Axons: "<<count<<std::endl;
        std::cout<<"number of synapse-sections: "<<(KyoukoRoot::m_segment->synapses.numberOfItems)<<std::endl;

        end = std::chrono::system_clock::now();
        const float duration = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        LOG_DEBUG("time: " + std::to_string(duration / 1000.0f) + '\n');
    }
}

} // namespace KyoukoMind
