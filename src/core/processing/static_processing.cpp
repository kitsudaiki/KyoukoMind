#include "static_processing.h"

#include <kyouko_root.h>

#include <core/objects/segment.h>
#include <core/objects/network_cluster.h>
#include <core/objects/output.h>

#include <core/processing/cpu/output_processing.h>
#include <core/processing/cpu/core_processing.h>

#include <core/processing/gpu/gpu_processing_uint.h>
#include <libKitsunemimiOpencl/gpu_interface.h>

StaticProcessing::StaticProcessing()
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
    KyoukoRoot::m_freezeState = false;

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

        start = std::chrono::system_clock::now();
        node_processing(synapseSegment->nodes,
                        synapseSegment->nodeBuffers,
                        synapseSegment->synapseBuffers,
                        synapseSegment->segmentMeta,
                        synapseSegment->synapseMetaData,
                        outputSegment->inputs);
        //m_gpu->node_processing();
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        //std::cout<<"node-time: "<<(timeValue / 1000.0f)<<" us"<<std::endl;

        updateCoreSynapses(synapseSegment->segmentMeta,
                           synapseSegment->synapseBuffers,
                           synapseSegment->synapseSections,
                           synapseSegment->nodes,
                           synapseSegment->synapseMetaData);

        start = std::chrono::system_clock::now();
        synapse_processing(synapseSegment->segmentMeta,
                           synapseSegment->synapseBuffers,
                           synapseSegment->synapseSections,
                           synapseSegment->nodes,
                           synapseSegment->nodeBricks,
                           synapseSegment->nodeBuffers,
                           synapseSegment->randomValues,
                           synapseSegment->synapseMetaData,
                           &KyoukoRoot::m_networkCluster->networkMetaData);
        //m_gpu->synapse_processing();
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        //std::cout<<"synapse-time: "<<(timeValue / 1000.0f)<<" us"<<std::endl;

        //KyoukoRoot::m_root->m_networkManager->executeStep();
    }

    output_node_processing(outputSegment->outputSynapseSections,
                           outputSegment->inputs,
                           outputSegment->outputs,
                           outputSegment->segmentMeta,
                           &KyoukoRoot::m_networkCluster->networkMetaData,
                           outputSegment->outputMetaData);
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
        for(uint32_t i = 0; i < 2400; i++) {
            inputNodes[i].weight = 0.0f;
        }

        executeStep(1);

        for(uint32_t i = 0; i < 2400; i++) {
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
        KyoukoRoot::m_freezeState = true;
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
        output_learn_step(outputSegment->outputSynapseSections,
                          outputSegment->inputs,
                          outputSegment->outputs,
                          outputSegment->segmentMeta,
                          outputSegment->randomValues,
                          &KyoukoRoot::m_networkCluster->networkMetaData,
                          outputSegment->outputMetaData);
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
        KyoukoRoot::m_freezeState = true;
        cluster->networkMetaData.lerningValue = 100000.0f;
        executeStep(1);
    }

    if(check != 0) {
        return false;
    }

    return true;
}
