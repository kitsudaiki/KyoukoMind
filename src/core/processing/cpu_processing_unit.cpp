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
CpuProcessingUnit::processNetworkCluster(NetworkCluster *cluster)
{
    for(AbstractSegment* segment : cluster->allSegments)
    {
        if(segment != nullptr)
        {
            switch(segment->getType())
            {
                case DYNAMIC_SEGMENT:
                    processDynamicSegment(static_cast<DynamicSegment*>(segment));
                    break;

                case INPUT_SEGMENT:
                    processInputSegment(static_cast<InputSegment*>(segment));
                    break;

                case OUTPUT_SEGMENT:
                    processOutputSegment(static_cast<OutputSegment*>(segment));
                    break;

                default:
                    break;
            }

            segment->finishSegment();
        }
    }
}

void
CpuProcessingUnit::processDynamicSegment(DynamicSegment* segment)
{
    segment->segmentSettings->doLearn = 1;

    prcessDynamicSegment(segment);
    hardenSegment(segment);
    rewightSegment(segment);

    segment->segmentSettings->doLearn = 0;
}

void
CpuProcessingUnit::processInputSegment(InputSegment* segment)
{
    prcessInputSegment(segment);

}

void
CpuProcessingUnit::processOutputSegment(OutputSegment* segment)
{
    prcessOutputSegment(segment);

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

