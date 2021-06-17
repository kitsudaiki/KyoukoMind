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

    for(uint32_t layerId = 0; layerId < synapseSegment->layer.size(); layerId++)
    {
        for(uint32_t brickId = 0; brickId < synapseSegment->layer.at(layerId).size(); brickId++)
        {
            Brick* brick = synapseSegment->layer.at(layerId).at(brickId);
            /*nodeProcessingMultiThread(brick,
                                      synapseSegment->nodes,
                                      synapseSegment->synapseBuffers,
                                      synapseSegment->synapseMetaData);
            processSynapseBuffer(synapseSegment->nodes,
                                 synapseSegment->segmentMeta,
                                 synapseSegment->synapseSections,
                                 synapseSegment->synapseBuffers,
                                 synapseSegment->nodeBricks,
                                 KyoukoRoot::m_networkCluster->randomValues,
                                 synapseSegment->synapseMetaData,
                                 &KyoukoRoot::m_networkCluster->networkMetaData);*/

            nodeProcessingSingleThread(brick,
                                       synapseSegment->nodes,
                                       synapseSegment->segmentMeta,
                                       synapseSegment->synapseSections,
                                       synapseSegment->nodeBricks,
                                       KyoukoRoot::m_networkCluster->randomValues,
                                       synapseSegment->synapseMetaData,
                                       &KyoukoRoot::m_networkCluster->networkMetaData);
        }
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
        int16_t timeout = 3;
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

    backpropagateOutput(synapseSegment->segmentMeta,
                        synapseSegment->nodes,
                        synapseSegment->outputNodes);

    int32_t layerId = synapseSegment->layer.size() - 2;

    for(uint32_t brickId = 0; brickId < synapseSegment->layer.at(layerId).size(); brickId++)
    {
        Brick* brick = synapseSegment->layer.at(layerId).at(brickId);
        correctNewOutputSynapses(brick,
                                 synapseSegment->nodes,
                                 synapseSegment->synapseSections);
    }

    for(layerId = synapseSegment->layer.size() - 2; layerId >= 0; layerId--)
    {
        for(uint32_t brickId = 0; brickId < synapseSegment->layer.at(layerId).size(); brickId++)
        {
            Brick* brick = synapseSegment->layer.at(layerId).at(brickId);
            backpropagateNodes(brick,
                               synapseSegment->nodes,
                               synapseSegment->synapseSections);
        }
    }

    hardenSynapses(synapseSegment->nodes,
                   synapseSegment->synapseSections,
                   synapseSegment->segmentMeta);
}

