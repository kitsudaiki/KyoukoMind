/**
 * @file        single_thread_processing_static.cpp
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

#include "single_thread_processing_static.h"

#include <core/processing/cpu/segment_processing.h>
#include <core/processing/static_processing/gpu_processing_static.h>
#include <core/objects/network_cluster.h>

SingleThreadProcessingStatic::SingleThreadProcessingStatic()
    : StaticProcessing()
{
}

/**
 * @brief SingleThreadProcessingStatic::executeStep
 */
void
SingleThreadProcessingStatic::executeStep()
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
SingleThreadProcessingStatic::reductionLearning()
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
SingleThreadProcessingStatic::updateLearning()
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

