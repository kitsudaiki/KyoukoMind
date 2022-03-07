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

#include <core/segments/dynamic_segment/dynamic_segment.h>
#include <core/segments/input_segment/input_segment.h>
#include <core/segments/output_segment/output_segment.h>

#include <core/cluster/cluster.h>

#include <kyouko_root.h>

#include <core/processing/segment_queue.h>

#include <core/segments/dynamic_segment/backpropagation.h>
#include <core/segments/dynamic_segment/processing.h>
#include <core/segments/dynamic_segment/reduction.h>

#include <core/segments/static_segment/backpropagation.h>
#include <core/segments/static_segment/processing.h>

#include <core/segments/output_segment/processing.h>

#include <core/segments/input_segment/processing.h>

/**
 * @brief constructor
 */
CpuProcessingUnit::CpuProcessingUnit(const std::string &threadName)
    : Kitsunemimi::Thread(threadName) {}

/**
 * @brief destructor
 */
CpuProcessingUnit::~CpuProcessingUnit() {}

/**
 * @brief run forward-propagation on a segment
 *
 * @param segment segment to process
 */
void
CpuProcessingUnit::learnSegmentForward(AbstractSegment* segment)
{
    switch(segment->getType())
    {
        case DYNAMIC_SEGMENT:
        {
            DynamicSegment* seg = static_cast<DynamicSegment*>(segment);
            seg->dynamicSegmentSettings->doLearn = 1;
            rewightDynamicSegment(*seg);
            prcessDynamicSegment(*seg);
            if(reductionCounter == 100) {
                reduceNodes(*seg);
            }
            reductionCounter++;
            seg->dynamicSegmentSettings->doLearn = 0;
            break;
        }
        case STATIC_SEGMENT:
        {
            StaticSegment* seg = static_cast<StaticSegment*>(segment);
            processStaticSegment(*seg);
            break;
        }
        case INPUT_SEGMENT:
        {
            InputSegment* seg = static_cast<InputSegment*>(segment);
            prcessInputSegment(*seg);
            break;
        }
        case OUTPUT_SEGMENT:
        {
            OutputSegment* seg = static_cast<OutputSegment*>(segment);
            seg->dynamicSegmentSettings->doLearn = 1;
            prcessOutputSegment(*seg);
            seg->dynamicSegmentSettings->doLearn = 0;

            break;
        }
        default:
            break;
    }
}

/**
 * @brief process segments
 *
 * @param segment segment to process
 */
void
CpuProcessingUnit::processSegment(AbstractSegment* segment)
{
    switch(segment->getType())
    {
        case DYNAMIC_SEGMENT:
        {
            DynamicSegment* seg = static_cast<DynamicSegment*>(segment);
            prcessDynamicSegment(*seg);
            break;
        }
        case STATIC_SEGMENT:
        {
            StaticSegment* seg = static_cast<StaticSegment*>(segment);
            processStaticSegment(*seg);
            break;
        }
        case INPUT_SEGMENT:
        {
            InputSegment* seg = static_cast<InputSegment*>(segment);
            prcessInputSegment(*seg);
            break;
        }
        case OUTPUT_SEGMENT:
        {
            OutputSegment* seg = static_cast<OutputSegment*>(segment);
            prcessOutputSegment(*seg);
            const uint32_t hightest = getHighestOutput(*seg);
            seg->parentCluster->setResultForActualCycle(hightest);
            break;
        }
        default:
            break;
    }
}

/**
 * @brief run loop to process all available segments
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

            // handle type of processing
            Cluster* clusterInterface = currentSegment->parentCluster;
            if(clusterInterface->getMode() == Cluster::LEARN_MODE) {
                learnSegmentForward(currentSegment);
            } else {
                processSegment(currentSegment);
            }

            // finish segment by sharing border-buffer and register in cluster
            currentSegment->finishSegment();
        }
        else
        {
            // if no segments are available then sleep
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
