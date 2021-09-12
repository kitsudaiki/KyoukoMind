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

#include <core/objects/segments/dynamic_segment.h>
#include <core/objects/segments/input_segment.h>
#include <core/objects/segments/output_segment.h>

#include <core/objects/network_cluster.h>

#include <kyouko_root.h>

#include <core/processing/cpu/dynamic_segment/backpropagation.h>
#include <core/processing/cpu/dynamic_segment/processing.h>
#include <core/processing/cpu/dynamic_segment/create_reduce.h>

#include <core/processing/cpu/output_segment/backpropagation.h>
#include <core/processing/cpu/output_segment/processing.h>

#include <core/processing/cpu/input_segment/processing.h>

CpuProcessingUnit::CpuProcessingUnit() {}

void
CpuProcessingUnit::learnNetworkCluster(NetworkCluster *cluster)
{
    for(uint32_t i = 0; i < cluster->allSegments.size(); i++)
    {
        AbstractSegment* segment = cluster->allSegments[i];
        if(segment != nullptr)
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

            segment->finishSegment();
        }
    }

    for(int32_t i = cluster->allSegments.size()-1; i >= 0; i--)
    {
        AbstractSegment* segment = cluster->allSegments[i];
        if(segment != nullptr)
        {
            switch(segment->getType())
            {
                case DYNAMIC_SEGMENT:
                {
                    DynamicSegment* seg = static_cast<DynamicSegment*>(segment);
                    seg->segmentSettings->doLearn = 1;
                    rewightSegment(seg);
                    seg->segmentSettings->doLearn = 0;
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

            segment->finishSegment();
        }
    }
}

void
CpuProcessingUnit::processNetworkCluster(NetworkCluster *cluster)
{
    for(AbstractSegment* segment : cluster->allSegments)
    {
        if(segment != nullptr)
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
                    break;
                }
                default:
                    break;
            }

            segment->finishSegment();
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

