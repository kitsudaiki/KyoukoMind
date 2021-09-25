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

#include "cpu_processing_unit.h"

#include <core/structure/segments/dynamic_segment.h>
#include <core/structure/segments/input_segment.h>
#include <core/structure/segments/output_segment.h>

#include <core/orchestration/task_queue.h>
#include <core/orchestration/cluster_interface.h>
#include <core/structure/network_cluster.h>

#include <kyouko_root.h>

#include <core/processing/segment_queue.h>

#include <core/processing/cpu/dynamic_segment/backpropagation.h>
#include <core/processing/cpu/dynamic_segment/processing.h>
#include <core/processing/cpu/dynamic_segment/create_reduce.h>

#include <core/processing/cpu/output_segment/backpropagation.h>
#include <core/processing/cpu/output_segment/processing.h>

#include <core/processing/cpu/input_segment/processing.h>

/**
 * @brief CpuProcessingUnit::CpuProcessingUnit
 */
CpuProcessingUnit::CpuProcessingUnit(int coreId)
    : Kitsunemimi::Thread(coreId) {}

/**
 * @brief CpuProcessingUnit::~CpuProcessingUnit
 */
CpuProcessingUnit::~CpuProcessingUnit() {}

/**
 * @brief CpuProcessingUnit::learnNetworkCluster
 * @param cluster
 */
void
CpuProcessingUnit::learnSegmentForward(AbstractSegment* segment)
{
    switch(segment->getType())
    {
        case DYNAMIC_SEGMENT:
        {
            DynamicSegment* seg = static_cast<DynamicSegment*>(segment);
            seg->segmentSettings->doLearn = 1;
            prcessDynamicSegment(seg);
            hardenSegment(seg);
            seg->segmentSettings->doLearn = 0;
            break;
        }
        case INPUT_SEGMENT:
        {
            InputSegment* seg = static_cast<InputSegment*>(segment);
            prcessInputSegment(seg);
            break;
        }
        case OUTPUT_SEGMENT:
        {
            OutputSegment* seg = static_cast<OutputSegment*>(segment);
            prcessOutputSegment(seg);
            break;
        }
        default:
            break;
    }
}

/**
 * @brief CpuProcessingUnit::learnSegmentBackward
 * @param segment
 */
void
CpuProcessingUnit::learnSegmentBackward(AbstractSegment* segment)
{
    switch(segment->getType())
    {
        case DYNAMIC_SEGMENT:
        {
            DynamicSegment* seg = static_cast<DynamicSegment*>(segment);
            rewightSegment(seg);
            break;
        }
        case OUTPUT_SEGMENT:
        {
            OutputSegment* seg = static_cast<OutputSegment*>(segment);
            backpropagateOutput(seg);
            break;
        }
        default:
            break;
    }
}

/**
 * @brief CpuProcessingUnit::processNetworkCluster
 * @param cluster
 */
void
CpuProcessingUnit::processSegment(AbstractSegment* segment)
{
    switch(segment->getType())
    {
        case DYNAMIC_SEGMENT:
        {
            DynamicSegment* seg = static_cast<DynamicSegment*>(segment);
            prcessDynamicSegment(seg);
            break;
        }
        case INPUT_SEGMENT:
        {
            InputSegment* seg = static_cast<InputSegment*>(segment);
            prcessInputSegment(seg);
            break;
        }
        case OUTPUT_SEGMENT:
        {
            OutputSegment* seg = static_cast<OutputSegment*>(segment);
            prcessOutputSegment(seg);
            const uint32_t hightest = getHighestOutput(seg);
            seg->parentCluster->setResultForActualCycle(hightest);
            break;
        }
        default:
            break;
    }
}

/**
 * @brief CpuProcessingUnit::run
 */
void
CpuProcessingUnit::run()
{
    AbstractSegment* currentSegment = nullptr;

    while(m_abort == false)
    {
        currentSegment = KyoukoRoot::m_segmentQueue->getSegmentFromQueue();
        if(currentSegment != nullptr)
        {
            // check if segment is ready, else requeue
            if(currentSegment->isReady() == false)
            {
                KyoukoRoot::m_segmentQueue->addSegmentToQueue(currentSegment);
                continue;
            }

            // reset input ready status
            for(uint8_t side = 0; side < 12; side++) {
                currentSegment->segmentNeighbors->neighbors[side].inputReady = false;
            }

            ClusterInterface* clusterInterface = currentSegment->parentCluster;
            if(clusterInterface->getMode() == LEARN_FORWARD_MODE) {
                learnSegmentForward(currentSegment);
            } else if(clusterInterface->getMode() == LEARN_BACKWARD_MODE) {
                learnSegmentBackward(currentSegment);
            } else {
                processSegment(currentSegment);
            }

            currentSegment->finishSegment();
        }
        else
        {
            sleepThread(10000);
        }
    }
}

/**
 * @brief SingleThreadProcessingStatic::reductionLearning

void
CpuProcessingUnit::reductionLearning(DynamicSegment* synapseSegment)
{
    const float initError = calculateSegmentError(synapseSegment);
    float error = initError;

    if(initError > 0.1f)
    {
        int16_t timeout = 10;
        while(error >= initError
              && timeout >= 0)
        {
            reduceSegment(synapseSegment);
            execute(synapseSegment);
            error = calculateSegmentError(synapseSegment);

            timeout--;
        }
    }
}*/

