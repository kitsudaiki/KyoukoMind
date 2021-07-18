#include "cpu_processing_unit.h"

#include <core/objects/segment.h>
#include <core/objects/network_cluster.h>
#include <core/processing/cpu/segment_processing.h>

CpuProcessingUnit::CpuProcessingUnit()
{

}

/**
 * @brief StaticProcessing::learn
 * @return
 */
bool
CpuProcessingUnit::learn()
{
    Segment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;
    synapseSegment->synapseSettings->doLearn = 1;
    updateLearning();
    synapseSegment->synapseSettings->doLearn = 0;

    return true;
}

/**
 * @brief StaticProcessing::execute
 * @return
 */
bool
CpuProcessingUnit::execute()
{
    executeStep();
    return true;
}


/**
 * @brief SingleThreadProcessingStatic::executeStep
 */
void
CpuProcessingUnit::executeStep()
{
    Segment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;

    // learn until output-section
    processSegmentInput(synapseSegment);
    prcessSegmentNodes(synapseSegment);
    processSegmentOutput(synapseSegment);
}

/**
 * @brief SingleThreadProcessingStatic::reductionLearning
 */
void
CpuProcessingUnit::reductionLearning()
{
    Segment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;

    const float initError = calculateSegmentError(synapseSegment);
    float error = initError;

    if(initError > 0.1f)
    {
        int16_t timeout = 10;
        while(error >= initError
              && timeout >= 0)
        {
            reduceSegmentSynapses(synapseSegment);
            executeStep();
            error = calculateSegmentError(synapseSegment);

            timeout--;
        }
    }

    hardenSegment(synapseSegment);
}

/**
 * @brief SingleThreadProcessingStatic::updateLearning
 */
void
CpuProcessingUnit::updateLearning()
{
    Segment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;

    executeStep();

    if(reductionCounter < 1000)
    {
        reductionLearning();
        reductionCounter++;
    }

    rewightSegment(synapseSegment);
    hardenSegment(synapseSegment);
}