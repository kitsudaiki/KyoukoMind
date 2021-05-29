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

#include <libKitsunemimiCommon/threading/barrier.h>

#include <core/objects/segment.h>
#include <core/objects/network_cluster.h>
#include <core/objects/brick.h>

#include <core/processing/cpu/core_processing.h>
#include <core/processing/cpu/output_processing.h>
#include <core/processing/gpu/gpu_processing_uint.h>

#include <core/processing/cpu/output_processing.h>
#include <core/processing/cpu/core_processing.h>

#include <libKitsunemimiPersistence/logger/logger.h>

/**
 * @brief constructor
 */
CpuProcessingUnit::CpuProcessingUnit(const uint32_t threadId,
                                     const uint32_t maxThreads)
{
    m_threadId = threadId;
    m_maxThreads = maxThreads;
}

/**
 * @brief run cpu-unit
 */
void
CpuProcessingUnit::run()
{
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    float timeValue = 0.0f;
    CoreSegment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;
    OutputSegment* outputSegment = KyoukoRoot::m_networkCluster->outputSegment;

    while(m_abort == false)
    {
        startBarrier->triggerBarrier();

        switch(m_currentTask)
        {
            case OUTPUT_LEARN:
            {
                output_learn_step(outputSegment->outputSynapseSections,
                                  outputSegment->inputs,
                                  outputSegment->outputs,
                                  outputSegment->segmentMeta,
                                  KyoukoRoot::m_networkCluster->randomValues,
                                  &KyoukoRoot::m_networkCluster->networkMetaData,
                                  outputSegment->outputMetaData,
                                  m_threadId,
                                  m_maxThreads);
                break;
            }

            case NODE_PROCESSING:
            {
                node_processing(synapseSegment->nodes,
                                synapseSegment->nodeBuffers,
                                synapseSegment->synapseBuffers,
                                synapseSegment->segmentMeta,
                                synapseSegment->synapseMetaData,
                                outputSegment->inputs,
                                m_threadId,
                                m_maxThreads);
                break;
            }
            case UPDATE_CORE:
            {
                updateCoreSynapses(synapseSegment->segmentMeta,
                                   synapseSegment->synapseBuffers,
                                   synapseSegment->synapseSections,
                                   synapseSegment->nodes,
                                   synapseSegment->synapseMetaData,
                                   m_threadId,
                                   m_maxThreads);
                break;
            }
            case CORE_PROCESSING:
            {
                synapse_processing(synapseSegment->segmentMeta,
                                   synapseSegment->synapseBuffers,
                                   synapseSegment->synapseSections,
                                   synapseSegment->nodes,
                                   synapseSegment->nodeBricks,
                                   synapseSegment->nodeBuffers,
                                   KyoukoRoot::m_networkCluster->randomValues,
                                   synapseSegment->synapseMetaData,
                                   &KyoukoRoot::m_networkCluster->networkMetaData,
                                   m_threadId,
                                   m_maxThreads);
                break;
            }
            case OUTPUT_PROCESSING:
            {
                output_node_processing(outputSegment->outputSynapseSections,
                                       outputSegment->inputs,
                                       outputSegment->outputs,
                                       outputSegment->segmentMeta,
                                       &KyoukoRoot::m_networkCluster->networkMetaData,
                                       outputSegment->outputMetaData,
                                       m_threadId,
                                       m_maxThreads);
                break;
            }

            case NOTHING:
            {
                sleepThread(10000);
                break;
            }
        }

        m_currentTask = NOTHING;

        endBarrier->triggerBarrier();
    }
}

void CpuProcessingUnit::setTask(const ThreadTask newTask)
{
    m_currentTask = newTask;
}
