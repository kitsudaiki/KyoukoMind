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

void
SingleThreadProcessingStatic::executeStep(const uint32_t runs)
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
            node_processing(synapseSegment->nodes,
                            synapseSegment->nodeBuffers,
                            synapseSegment->segmentMeta,
                            synapseSegment->synapseSections,
                            synapseSegment->nodeBricks,
                            KyoukoRoot::m_networkCluster->randomValues,
                            synapseSegment->synapseMetaData,
                            &KyoukoRoot::m_networkCluster->networkMetaData,
                            synapseSegment->layer.at(layerId).at(brickId)->nodePos,
                            synapseSegment->segmentMeta->numberOfNodesPerBrick);
        }
    }

    processOutputNodes(synapseSegment->nodes,
                       synapseSegment->outputNodes,
                       synapseSegment->segmentMeta);
}

void
SingleThreadProcessingStatic::runBackpropagation()
{
    CoreSegment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;

    backpropagateOutput(synapseSegment->segmentMeta,
                        synapseSegment->nodes,
                        synapseSegment->outputNodes);

    int32_t layerId = synapseSegment->layer.size() - 2;

    for(uint32_t brickId = 0; brickId < synapseSegment->layer.at(layerId).size(); brickId++)
    {
        correctNewOutputSynapses(synapseSegment->nodes,
                                 synapseSegment->synapseSections,
                                 synapseSegment->layer.at(layerId).at(brickId)->nodePos,
                                 synapseSegment->segmentMeta->numberOfNodesPerBrick);
    }

    for(layerId = synapseSegment->layer.size() - 2; layerId >= 0; layerId--)
    {
        for(uint32_t brickId = 0; brickId < synapseSegment->layer.at(layerId).size(); brickId++)
        {
            backpropagateNodes(synapseSegment->nodes,
                               synapseSegment->synapseSections,
                               synapseSegment->layer.at(layerId).at(brickId)->nodePos,
                               synapseSegment->segmentMeta->numberOfNodesPerBrick);
        }
    }
}

void SingleThreadProcessingStatic::reductionLearning(const uint32_t runs)
{
    CoreSegment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;

    // learn until output-section
    processInputNodes(synapseSegment->nodes,
                      synapseSegment->inputNodes,
                      synapseSegment->segmentMeta);

    const uint32_t runCount = runs;
    for(uint32_t i = 0; i < runCount; i++)
    {
    }
}

void
SingleThreadProcessingStatic::updateLearning(const uint32_t runs)
{
    CoreSegment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;

    executeStep(runs);

    const float initError = calcTotalError(synapseSegment->outputNodes,
                                           synapseSegment->segmentMeta);
    float error = initError;

    /*if(initError > 0.01f)
    {
        int16_t timeout = 3;
        while(error >= initError
              && timeout >= 0)
        {
            std::cout<<"------- error: "<<error<<std::endl;
            reduceCoreSynapses(synapseSegment->segmentMeta,
                               synapseSegment->synapseSections,
                               synapseSegment->nodes);
            executeStep(runs);
            error = calcTotalError(synapseSegment->outputNodes,
                                   synapseSegment->segmentMeta);

            timeout--;
        }
    }

    std::cout<<"------- error: "<<error<<std::endl;*/

    if(error > 0.01f) {
        runBackpropagation();
    }

    hardenSynapses(synapseSegment->nodes,
                   synapseSegment->synapseSections,
                   synapseSegment->segmentMeta);
}
