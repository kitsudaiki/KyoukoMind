#include "learner.h"

#include <kyouko_root.h>

#include <core/objects/segment.h>
#include <core/objects/network_cluster.h>
#include <core/objects/output.h>

#include <core/processing/cpu/output_synapse_processing.h>
#include <core/processing/cpu/synapse_processing.h>

#include <core/processing/gpu/gpu_processing_uint.h>
#include <libKitsunemimiOpencl/gpu_interface.h>

Learner::Learner()
{
    m_gpuHandler = new Kitsunemimi::Opencl::GpuHandler();
    assert(m_gpuHandler->m_interfaces.size() >= 1);
    m_gpu = new GpuProcessingUnit(m_gpuHandler->m_interfaces.at(0));
    assert(m_gpu->initializeGpu(KyoukoRoot::m_networkCluster));
}

/**
 * @brief Lerner::learnStep
 * @return
 */
uint32_t
Learner::learnStep(uint32_t label)
{
    OutputSegment* outputSegment = KyoukoRoot::m_networkCluster->outputSegment;
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;
    uint32_t timeout = 50;
    InputNode* inputNodes = cluster->synapseSegment->inputNodes;

    cluster->networkMetaData.doLearn = 1;

    OutputInput* outputInputs = cluster->outputSegment->inputs;
    for(uint32_t i = 0; i < cluster->outputSegment->segmentMeta->numberOfInputs; i++) {
        outputInputs[i].isNew = 0;
    }

    //----------------------------------------------------------------------------------------------
    // learn phase 1
    timeout = 5;
    uint32_t updateVals = 0;
    uint32_t tempVal = 0;
    do
    {
        for(uint32_t i = 0; i < 2400; i++) {
            inputNodes[i].weight = 0.0f;
        }

        executeStep();

        for(uint32_t i = 0; i < 2400; i++) {
            inputNodes[i].weight = buffer[i];
        }

        executeStep();
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
        KyoukoRoot::m_freezeState = true;
        cluster->networkMetaData.lerningValue = 100000.0f;

        executeStep();

        finishStep();

        return true;
    }

    //----------------------------------------------------------------------------------------------
    // learn phase 2
    timeout = 50;
    uint32_t check = 0;
    do
    {
        /*output_learn_step(outputSegment->outputSynapseSections,
                          outputSegment->inputs,
                          outputSegment->outputs,
                          outputSegment->segmentMeta,
                          outputSegment->randomValues,
                          &KyoukoRoot::m_networkCluster->networkMetaData,
                          outputSegment->outputMetaData);*/
        m_gpu->output_learn_step();
        timeout--;
        check = checkOutput(outputSegment->segmentMeta, outputSegment->outputs);
    }
    while(check > 0
          && timeout > 0);
    std::cout<<"###################################################: "<<check<<std::endl;

    // if desired state was reached, than freeze lerned state
    if(check == 0)
    {
        KyoukoRoot::m_freezeState = true;
        cluster->networkMetaData.lerningValue = 100000.0f;
        executeStep();
    }

    //----------------------------------------------------------------------------------------------

    finishStep();

    return tempVal;
}

/**
 * @brief Lerner::executeStep
 */
void
Learner::executeStep()
{
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;

    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    float timeValue = 0.0f;
    SynapseSegment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;
    OutputSegment* outputSegment = KyoukoRoot::m_networkCluster->outputSegment;

    // learn until output-section
    const uint32_t runCount = cluster->initMetaData.layer + 2;
    for(uint32_t i = 0; i < runCount; i++)
    {
        start = std::chrono::system_clock::now();
        /*node_processing(synapseSegment->nodes,
                        synapseSegment->nodeBuffers,
                        synapseSegment->inputNodes,
                        synapseSegment->synapseBuffers,
                        synapseSegment->segmentMeta,
                        synapseSegment->synapseMetaData,
                        outputSegment->inputs);*/
        m_gpu->node_processing();
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        //std::cout<<"node-time: "<<(timeValue / 1000.0f)<<" us"<<std::endl;

        start = std::chrono::system_clock::now();
        /*synapse_processing(synapseSegment->segmentMeta,
                           synapseSegment->synapseBuffers,
                           synapseSegment->synapseSections,
                           synapseSegment->nodes,
                           synapseSegment->nodeBricks,
                           synapseSegment->nodeBuffers,
                           synapseSegment->randomValues,
                           synapseSegment->synapseMetaData,
                           &KyoukoRoot::m_networkCluster->networkMetaData);*/
        m_gpu->synapse_processing();
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        //std::cout<<"synapse-time: "<<(timeValue / 1000.0f)<<" us"<<std::endl;

        //KyoukoRoot::m_root->m_networkManager->executeStep();
    }

    /*output_node_processing(outputSegment->outputSynapseSections,
                           outputSegment->inputs,
                           outputSegment->outputs,
                           outputSegment->segmentMeta,
                           &KyoukoRoot::m_networkCluster->networkMetaData,
                           outputSegment->outputMetaData);*/
    m_gpu->output_node_processing();
}

/**
 * @brief Lerner::finishStep
 */
void
Learner::finishStep()
{
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;

    cluster->networkMetaData.doLearn = 0;
    cluster->networkMetaData.lerningValue = 0.0f;

    KyoukoRoot::m_freezeState = false;

    m_gpu->finish();
    /*OutputInput* outputInputs = cluster->outputSegment->inputs;
    for(uint32_t i = 0; i < cluster->outputSegment->segmentMeta->numberOfInputs; i++) {
        outputInputs[i].isNew = 0;
    }*/
}

/**
 * @brief Learner::output_precheck
 * @param segmentMeta
 * @param outputs
 * @return
 */
uint32_t
Learner::checkOutput(OutputSegmentMeta* segmentMeta,
                     Output* outputs)
{
    uint32_t updateVals = 0;

    for(uint32_t o = 0; o < segmentMeta->numberOfOutputs; o++)
    {
        Output* out = &outputs[o];
        if(out->shouldValue == 0.0f
                && out->outputValue <= out->shouldValue + 10.0f)
        {
            continue;
        }

        if(out->shouldValue > 0.0f
                && out->outputValue >= out->shouldValue - 10.0f)
        {
            continue;
        }

        updateVals++;
    }

    return updateVals;
}
