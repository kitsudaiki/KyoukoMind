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

#include <core/processing/cpu/processing.h>
#include <core/processing/cpu/io.h>
#include <core/processing/cpu/backpropagation.h>
#include <core/processing/static_processing/gpu_processing_static.h>
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
    CoreSegment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;

    // learn until output-section
    processInputNodes(synapseSegment->nodes,
                      synapseSegment->inputNodes,
                      synapseSegment->segmentMeta);

    const uint32_t numberOfBricks = synapseSegment->segmentMeta->numberOfNodeBricks;
    for(uint32_t pos = 0; pos < numberOfBricks; pos++)
    {
        Brick* brick = synapseSegment->brickOrder[pos];
        nodeProcessingSingleThread(brick,
                                   synapseSegment->nodes,
                                   synapseSegment->synapseSections,
                                   synapseSegment->nodeBricks,
                                   KyoukoRoot::m_networkCluster->randomValues,
                                   synapseSegment->synapseMetaData,
                                   &KyoukoRoot::m_networkCluster->networkMetaData);
    }

    processOutputNodes(synapseSegment->nodes,
                       synapseSegment->outputNodes,
                       synapseSegment->segmentMeta);
}

/**
 * @brief SingleThreadProcessingStatic::reductionLearning
 */
void
SingleThreadProcessingStatic::reductionLearning()
{
    CoreSegment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;

    const float initError = calcTotalError(synapseSegment->outputNodes,
                                           synapseSegment->segmentMeta);
    float error = initError;

    if(initError > 0.1f)
    {
        int16_t timeout = 10;
        while(error >= initError
              && timeout >= 0)
        {
            reduceCoreSynapses(synapseSegment->segmentMeta,
                               synapseSegment->synapseSections,
                               synapseSegment->nodes);
            executeStep();
            error = calcTotalError(synapseSegment->outputNodes,
                                   synapseSegment->segmentMeta);

            timeout--;
        }
    }

    hardenSynapses(synapseSegment->nodes,
                   synapseSegment->synapseSections,
                   synapseSegment->segmentMeta);
}

/**
 * @brief SingleThreadProcessingStatic::updateLearning
 */
void
SingleThreadProcessingStatic::updateLearning()
{
    CoreSegment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;

    executeStep();

    if(reductionCounter < 0)
    {
        reductionLearning();
        reductionCounter++;
    }

    backpropagateOutput(synapseSegment->segmentMeta,
                        synapseSegment->nodes,
                        synapseSegment->outputNodes);

    const uint32_t numberOfBricks = synapseSegment->segmentMeta->numberOfNodeBricks;
    for(int32_t pos = numberOfBricks - 1; pos >= 0; pos--)
    {
        Brick* brick = synapseSegment->brickOrder[pos];
        if(brick->isOutputBrick)
        {
            correctNewOutputSynapses(brick,
                                     synapseSegment->nodes,
                                     synapseSegment->synapseSections);
        }
    }

    for(int32_t pos = numberOfBricks - 1; pos >= 0; pos--)
    {
        Brick* brick = synapseSegment->brickOrder[pos];
        backpropagateNodes(brick,
                           synapseSegment->nodes,
                           synapseSegment->synapseSections);
    }

    hardenSynapses(synapseSegment->nodes,
                   synapseSegment->synapseSections,
                   synapseSegment->segmentMeta);
}

