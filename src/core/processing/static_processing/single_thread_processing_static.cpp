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

#include <core/processing/cpu/core_processing.h>

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

    const uint32_t runCount = runs;
    for(uint32_t i = 0; i < runCount; i++)
    {
        node_processing(synapseSegment->nodes,
                        synapseSegment->nodeBuffers,
                        synapseSegment->segmentMeta,
                        synapseSegment->synapseSections,
                        synapseSegment->nodeBricks,
                        KyoukoRoot::m_networkCluster->randomValues,
                        synapseSegment->synapseMetaData,
                        &KyoukoRoot::m_networkCluster->networkMetaData,
                        0,
                        1);
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

    // learn until output-section
    processInputNodes(synapseSegment->nodes,
                      synapseSegment->inputNodes,
                      synapseSegment->segmentMeta);

    const uint32_t runCount = runs;
    for(uint32_t i = 0; i < runCount; i++)
    {
        node_processing(synapseSegment->nodes,
                        synapseSegment->nodeBuffers,
                        synapseSegment->segmentMeta,
                        synapseSegment->synapseSections,
                        synapseSegment->nodeBricks,
                        KyoukoRoot::m_networkCluster->randomValues,
                        synapseSegment->synapseMetaData,
                        &KyoukoRoot::m_networkCluster->networkMetaData,
                        0,
                        1);

        updateCoreSynapses(synapseSegment->segmentMeta,
                           synapseSegment->synapseSections,
                           synapseSegment->nodes,
                           synapseSegment->synapseMetaData,
                           0,
                           1);
    }
}

void
SingleThreadProcessingStatic::outputLearn()
{


}
