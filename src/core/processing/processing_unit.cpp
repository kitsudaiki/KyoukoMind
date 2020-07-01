/**
 *  @file    processing_unit.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#include <core/processing/processing_unit.h>
#include <kyouko_root.h>

#include <core/global_values_handler.h>

#include <core/objects/brick.h>
#include <core/objects/container_definitions.h>

#include <core/methods/neighbor_methods.h>

#include <core/processing/methods/edge_container_processing.h>
#include <core/processing/methods/brick_processing.h>
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

        Brick* brick = KyoukoRoot::m_queue->getFromQueue();
        if(brick == nullptr)
        {
            GlobalValues globalValues = KyoukoRoot::m_globalValuesHandler->getGlobalValues();
            globalValues.globalLearningTemp = 0.0f;
            globalValues.globalMemorizingTemp = 0.0f;
            KyoukoRoot::m_globalValuesHandler->setGlobalValues(globalValues);

            end = std::chrono::system_clock::now();
            const float duration = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
            LOG_DEBUG("time: " + std::to_string(duration / 1000.0f) + '\n');

            /*if(USE_GPU)
            {
                // run process on gpu
                start = std::chrono::system_clock::now();
                runOnGpu(*segment);
                end = std::chrono::system_clock::now();
                const float gpu1 = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
                LOG_DEBUG("gpu run-time: " + std::to_string(gpu1 / 1000.0f) + '\n');

                // copy result from gpu to host
                start = std::chrono::system_clock::now();
                copyAxonsFromGpu(*segment);
                end = std::chrono::system_clock::now();
                const float gpu2 = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
                LOG_DEBUG("gpu copy-time: " + std::to_string(gpu2 / 1000.0f) + '\n');

                // debug-output
                uint32_t count = 0;
                AxonTransfer* axons = static_cast<AxonTransfer*>(segment->axonEdges.buffer.data);
                for(uint32_t i = 0; i < segment->axonEdges.numberOfItems; i++)
                {
                    if(axons[i].weight != 0.0f) {
                        count++;
                    }
                }
                std::cout<<"number of active Axons: "<<count<<std::endl;
            }*/

            // block thread until next cycle if queue is empty
            blockThread();

            start = std::chrono::system_clock::now();
        }
        else
        {
            initCycle(brick);

            // main-processing
            brick->globalValues = KyoukoRoot::m_globalValuesHandler->getGlobalValues();
            processIncomingMessages(*segment, *brick);

            // write output
            if(brick->isOutputBrick == 1) {
                writeClientOutput(*segment, *brick, m_clientBuffer);
            }
            writeMonitoringOutput(*brick, m_monitoringBuffer);

            // finish current block
            finishCycle(brick);
        }
    }
}

} // namespace KyoukoMind
