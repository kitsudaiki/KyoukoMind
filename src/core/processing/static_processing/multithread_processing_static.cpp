/**
 * @file        multithread_processing_static.cpp
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

#include "multithread_processing_static.h"

#include <core/processing/cpu/core_processing.h>
#include <core/processing/processing_unit_handler.h>

#include <libKitsunemimiCommon/threading/barrier.h>

MultiThreadProcessingStatic::MultiThreadProcessingStatic()
{
    m_startBarrier = new Kitsunemimi::Barrier(9);
    m_endBarrier = new Kitsunemimi::Barrier(9);

    m_processingUnitHandler = new ProcessingUnitHandler();
    m_processingUnitHandler->initProcessingUnits(m_startBarrier,
                                                 m_endBarrier,
                                                 8);
}

void
MultiThreadProcessingStatic::executeStep(const uint32_t runs)
{
    CoreSegment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;

    const uint32_t runCount = runs;
    for(uint32_t i = 0; i < runCount; i++)
    {
        processInputNodes(synapseSegment->nodes,
                          synapseSegment->inputNodes,
                          synapseSegment->segmentMeta);

        m_processingUnitHandler->shareNewTask(NODE_PROCESSING);
        m_startBarrier->triggerBarrier();
        m_endBarrier->triggerBarrier();

        m_processingUnitHandler->shareNewTask(UPDATE_CORE);
        m_startBarrier->triggerBarrier();
        m_endBarrier->triggerBarrier();

        m_processingUnitHandler->shareNewTask(CORE_PROCESSING);
        m_startBarrier->triggerBarrier();
        m_endBarrier->triggerBarrier();
    }

    m_processingUnitHandler->shareNewTask(OUTPUT_PROCESSING);
    m_startBarrier->triggerBarrier();
    m_endBarrier->triggerBarrier();
}

void
MultiThreadProcessingStatic::reductionLearning(const uint32_t runs)
{

}

void
MultiThreadProcessingStatic::updateLearning(const uint32_t runs)
{

}
