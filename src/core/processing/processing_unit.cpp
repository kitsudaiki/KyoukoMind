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

#include <core/brick.h>
#include <core/processing/objects/container_definitions.h>

#include <core/processing/methods/message_processing.h>
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

            if(USE_GPU)
            {
                // copy transfer-edges to gpu
                start = std::chrono::system_clock::now();
                copyEdgesToGpu(*segment);
                end = std::chrono::system_clock::now();
                const float gpu0 = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
                LOG_DEBUG("time copy to gpu: " + std::to_string(gpu0 / 1000.0f) + '\n');

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
                LOG_DEBUG("time copy from gpu: " + std::to_string(gpu2 / 1000.0f) + '\n');

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
            }

            segment->resetTransferPos();

            // block thread until next cycle if queue is empty
            blockThread();

            start = std::chrono::system_clock::now();
        }
        else
        {
            brick->initCycle();

            if(brick->isNodeBrick)
            {
                // process axons coming from the gpu
                AxonTransfer* axons = static_cast<AxonTransfer*>(segment->axonEdges.buffer.data);
                for(uint32_t i = 0; i < NUMBER_OF_NODES_PER_BRICK; i++)
                {
                    const uint64_t offset = brick->nodeBrickId * NUMBER_OF_NODES_PER_BRICK;
                    if(axons[offset + i].weight != 0.0f)
                    {
                        AxonEdgeContainer tempContainer;
                        tempContainer.weight = axons[offset + i].weight;
                        tempContainer.targetBrickPath = axons[offset + i].path;
                        tempContainer.targetAxonId = axons[offset + i].targetId;
                        processAxon(*segment, *brick, tempContainer);
                        LOG_ERROR("poi: " + std::to_string(tempContainer.weight));
                    }
                }
            }

            // main-processing
            brick->globalValues = KyoukoRoot::m_globalValuesHandler->getGlobalValues();
            processIncomingMessages(*segment, *brick);

            // write output
            if(brick->isOutputBrick == 1) {
                brick->writeClientOutput(*segment, m_clientBuffer);
            }
            brick->writeMonitoringOutput(m_monitoringBuffer);

            // finish current block
            brick->finishCycle();
        }
    }
}

} // namespace KyoukoMind
