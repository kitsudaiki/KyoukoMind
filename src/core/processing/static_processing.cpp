/**
 * @file        static_processing.cpp
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

#include "static_processing.h"

#include <kyouko_root.h>

#include <core/objects/segment.h>
#include <core/objects/network_cluster.h>
#include <core/objects/output.h>

#include <core/processing/cpu/output_processing.h>
#include <core/processing/cpu/core_processing.h>
#include <core/processing/processing_unit_handler.h>

#include <core/processing/gpu/gpu_processing_uint.h>
#include <libKitsunemimiOpencl/gpu_interface.h>
#include <libKitsunemimiCommon/threading/barrier.h>

StaticProcessing::StaticProcessing(const bool useGpu)
{
    m_useGpu = useGpu;

    m_startBarrier = new Kitsunemimi::Barrier(9);
    m_endBarrier = new Kitsunemimi::Barrier(9);

    m_processingUnitHandler = new ProcessingUnitHandler();
    m_processingUnitHandler->initProcessingUnits(m_startBarrier,
                                                 m_endBarrier,
                                                 8);

    if(useGpu)
    {
        m_gpuHandler = new Kitsunemimi::Opencl::GpuHandler();
        assert(m_gpuHandler->m_interfaces.size() >= 1);
        m_gpu = new GpuProcessingUnit(m_gpuHandler->m_interfaces.at(0));
        assert(m_gpu->initializeGpu(KyoukoRoot::m_networkCluster));
    }
}

/**
 * @brief StaticProcessing::learn
 * @return
 */
bool
StaticProcessing::learn()
{
    return learnStep();
}

/**
 * @brief StaticProcessing::execute
 * @return
 */
bool
StaticProcessing::execute()
{
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;
    executeStep(cluster->initMetaData.layer + 2);
    return true;
}

/**
 * @brief Lerner::learnStep
 * @return
 */
bool
StaticProcessing::learnStep()
{
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;
    cluster->networkMetaData.doLearn = 1;

    OutputInput* outputInputs = cluster->outputSegment->inputs;
    for(uint32_t i = 0; i < cluster->outputSegment->segmentMeta->numberOfInputs; i++) {
        outputInputs[i].isNew = 0;
    }

    bool result = learnPhase1();
    if(result == false) {
        result = learnPhase2();
    }

    cluster->networkMetaData.doLearn = 0;
    cluster->networkMetaData.lerningValue = 0.0f;

    //m_gpu->finish();
    for(uint32_t i = 0; i < cluster->outputSegment->segmentMeta->numberOfInputs; i++) {
        outputInputs[i].isNew = 0;
    }

    return result;
}

/**
 * @brief Lerner::executeStep
 */
void
StaticProcessing::executeStep(const uint32_t runs)
{
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    float timeValue = 0.0f;
    CoreSegment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;
    OutputSegment* outputSegment = KyoukoRoot::m_networkCluster->outputSegment;

    // learn until output-section
    const uint32_t runCount = runs;
    for(uint32_t i = 0; i < runCount; i++)
    {
        processInputNodes(synapseSegment->nodes,
                          synapseSegment->inputNodes,
                          synapseSegment->segmentMeta);

        m_processingUnitHandler->shareNewTask(NODE_PROCESSING);
        m_startBarrier->triggerBarrier();
        m_endBarrier->triggerBarrier();

        //m_gpu->node_processing();
        m_processingUnitHandler->shareNewTask(UPDATE_CORE);
        m_startBarrier->triggerBarrier();
        m_endBarrier->triggerBarrier();

        m_processingUnitHandler->shareNewTask(CORE_PROCESSING);
        m_startBarrier->triggerBarrier();
        m_endBarrier->triggerBarrier();

        //m_gpu->synapse_processing();

    }

    m_processingUnitHandler->shareNewTask(OUTPUT_PROCESSING);
    m_startBarrier->triggerBarrier();
    m_endBarrier->triggerBarrier();

    //m_gpu->output_node_processing();
}

/**
 * @brief Learner::output_precheck
 * @param segmentMeta
 * @param outputs
 * @return
 */
uint32_t
StaticProcessing::checkOutput(OutputSegmentMeta* segmentMeta,
                              Output* outputs)
{
    uint32_t updateVals = 0;

    for(uint32_t o = 0; o < segmentMeta->numberOfOutputs; o++)
    {
        uint32_t updateVal = 1;
        Output* out = &outputs[o];
        updateVal -= out->shouldValue == 0.0f && out->outputValue <= out->shouldValue + 10.0f;
        updateVal -= out->shouldValue > 0.0f && out->outputValue >= out->shouldValue - 10.0f;
        updateVals += updateVal;
    }

    return updateVals;
}

/**
 * @brief Learner::learnPhase1
 */
bool
StaticProcessing::learnPhase1()
{
    OutputSegment* outputSegment = KyoukoRoot::m_networkCluster->outputSegment;
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;
    InputNode* inputNodes = cluster->synapseSegment->inputNodes;

    //----------------------------------------------------------------------------------------------
    // learn phase 1
    uint32_t timeout = 5;
    uint32_t updateVals = 0;
    uint32_t tempVal = 0;
    do
    {
        for(uint32_t i = 0; i < 1600; i++) {
            inputNodes[i].weight = 0.0f;
        }

        executeStep(1);

        for(uint32_t i = 0; i < 1600; i++) {
            inputNodes[i].weight = buffer[i];
        }

        executeStep(cluster->initMetaData.layer + 2);
        tempVal = checkOutput(outputSegment->segmentMeta, outputSegment->outputs);
        std::cout<<"++++++++++++++++++++++++++++++++++++++++++++++++++++++: "<<tempVal<<std::endl;

        if(tempVal < updateVals)
        {
            updateVals = tempVal;
            break;
        }
        updateVals = tempVal;

        timeout--;
    }
    while(updateVals != 0
          && timeout > 0);


    if(updateVals == 0)
    {
        cluster->networkMetaData.lerningValue = 100000.0f;

        executeStep(1);

        return true;
    }

    return false;
}

/**
 * @brief Learner::learnPhase2
 */
bool
StaticProcessing::learnPhase2()
{
    OutputSegment* outputSegment = KyoukoRoot::m_networkCluster->outputSegment;
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;

    uint32_t timeout = 10;
    uint32_t check = 0;
    do
    {
        m_processingUnitHandler->shareNewTask(OUTPUT_LEARN);
        m_startBarrier->triggerBarrier();
        m_endBarrier->triggerBarrier();
        //m_gpu->output_learn_step();
        timeout--;
        check = checkOutput(outputSegment->segmentMeta, outputSegment->outputs);
    }
    while(check > 0
          && timeout > 0);
    std::cout<<"###################################################: "<<check<<" : "<<timeout<<std::endl;

    // if desired state was reached, than freeze lerned state
    if(check == 0)
    {
        cluster->networkMetaData.lerningValue = 100000.0f;
        executeStep(1);
    }

    if(check != 0) {
        return false;
    }

    return true;
}
