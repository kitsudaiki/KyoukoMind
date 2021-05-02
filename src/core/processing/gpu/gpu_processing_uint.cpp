/**
 * @file        gpu_processing_uint.cpp
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

#include "gpu_processing_uint.h"

#include <synapse_node_processing.h>
#include <libKitsunemimiOpencl/gpu_interface.h>
#include <libKitsunemimiCommon/threading/barrier.h>

#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/network_cluster.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiAiCommon/metadata.h>


/**
 * @brief constructor
 *
 * @param gpuInterface interface-object for handle the connection to the gpu
 */
GpuProcessingUnit::GpuProcessingUnit(Kitsunemimi::Opencl::GpuInterface* gpuInterface)
{
    m_gpuHandler = new Kitsunemimi::Opencl::GpuHandler();
    assert(m_gpuHandler->m_interfaces.size() > 0);
    m_gpuInterface = m_gpuHandler->m_interfaces.at(0);
    m_gpuInterface = gpuInterface;
}

/**
 * @brief initialize gpu-kernel and -buffer
 *
 * @param segment segment with the buffer
 * @param numberOfBricks number of bricks to scale the number of working-groups
 *
 * @return true, if successfull, else false
 */
bool
GpuProcessingUnit::initializeGpu(NetworkCluster* cluster)
{
    const std::string processingCode(reinterpret_cast<char*>(synapse_node_processing_cl),
                                     synapse_node_processing_cl_len);

    // init worker-sizes
    oclData.numberOfWg.x = cluster->synapseSegment->segmentMeta->numberOfNodeBricks;
    // only 127 threads per group, instead of 128, because 1/128 of the local memory is used for
    // the meta-data
    oclData.threadsPerWg.x = 127;

    //==============================================================================================

    // fill buffer for nodes to map on gpu

    oclData.addBuffer("networkMetaData",    1, sizeof(Kitsunemimi::Ai::NetworkMetaData), true, &cluster->networkMetaData);

    OutputSegmentMeta* outputSegmentMeta = cluster->outputSegment->segmentMeta;
    oclData.addBuffer("outputSegmentMeta",     1,                                  sizeof(OutputSegmentMeta),               false, outputSegmentMeta);
    oclData.addBuffer("outputs",               outputSegmentMeta->numberOfOutputs, sizeof(Output),                          false, cluster->outputSegment->outputs);
    oclData.addBuffer("outputInputs",          outputSegmentMeta->numberOfInputs,  sizeof(OutputInput),                     false, cluster->outputSegment->inputs);
    oclData.addBuffer("outputSynapseSections", outputSegmentMeta->numberOfOutputs, sizeof(OutputSynapseSection),            false, cluster->outputSegment->outputSynapseSections);
    oclData.addBuffer("outputMetaData",        1,                                  sizeof(Kitsunemimi::Ai::OutputMetaData), false, cluster->outputSegment->outputMetaData);

    SynapseSegmentMeta* synapseSegmentMeta = cluster->synapseSegment->segmentMeta;
    oclData.addBuffer("synapseMetaData",    1,                                           sizeof(Kitsunemimi::Ai::SynapseMetaData), false, cluster->synapseSegment->synapseMetaData);
    oclData.addBuffer("synapseSegmentMeta", 1,                                           sizeof(SynapseSegmentMeta),               false, cluster->synapseSegment->segmentMeta);
    oclData.addBuffer("randomValues",       synapseSegmentMeta->numberOfRandomValues,    sizeof(uint32_t),                         false, cluster->synapseSegment->randomValues);
    oclData.addBuffer("nodeBuffers",        synapseSegmentMeta->numberOfNodes * 127,     sizeof(float),                            false, cluster->synapseSegment->nodeBuffers);
    oclData.addBuffer("bricks",             synapseSegmentMeta->numberOfNodeBricks,      sizeof(Brick),                            false, cluster->synapseSegment->nodeBricks);
    oclData.addBuffer("nodes",              synapseSegmentMeta->numberOfNodes,           sizeof(Node),                             false, cluster->synapseSegment->nodes);
    oclData.addBuffer("synapseSections",    synapseSegmentMeta->numberOfSynapseSections, sizeof(SynapseSection),                   false, cluster->synapseSegment->synapseSections);
    oclData.addBuffer("synapseBuffers",     synapseSegmentMeta->numberOfSynapseSections, sizeof(SynapseBuffer),                    false, cluster->synapseSegment->synapseBuffers);
    oclData.addBuffer("inputNodes",         synapseSegmentMeta->numberOfInputs,          sizeof(InputNode),                        true, cluster->synapseSegment->inputNodes);

    //==============================================================================================

    assert(m_gpuInterface->initCopyToDevice(oclData));

    //==============================================================================================

    // init kernel
    assert(m_gpuInterface->addKernel(oclData, "synapse_processing",  processingCode));
    assert(m_gpuInterface->addKernel(oclData, "node_processing",  processingCode));
    assert(m_gpuInterface->addKernel(oclData, "output_node_processing",  processingCode));
    assert(m_gpuInterface->addKernel(oclData, "output_learn_step",  processingCode));

    // bind buffer for node_processing kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "node_processing", "nodes"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "node_processing", "nodeBuffers"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "node_processing", "inputNodes"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "node_processing", "synapseBuffers"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "node_processing", "synapseSegmentMeta"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "node_processing", "synapseMetaData"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "node_processing", "outputInputs"));

    // bind buffer for synapse_processing kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "synapseSegmentMeta"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "synapseBuffers"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "synapseSections"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "nodes"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "bricks"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "nodeBuffers"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "randomValues"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "synapseMetaData"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "networkMetaData"));

    // bind buffer for output_node_processing kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "output_node_processing", "outputSynapseSections"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "output_node_processing", "outputInputs"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "output_node_processing", "outputs"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "output_node_processing", "outputSegmentMeta"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "output_node_processing", "networkMetaData"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "output_node_processing", "outputMetaData"));

    // bind buffer for output_learn_step kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "output_learn_step", "outputSynapseSections"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "output_learn_step", "outputInputs"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "output_learn_step", "outputs"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "output_learn_step", "outputSegmentMeta"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "output_learn_step", "randomValues"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "output_learn_step", "networkMetaData"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "output_learn_step", "outputMetaData"));

    //==============================================================================================

    // init local memory for the kernels
    assert(m_gpuInterface->getLocalMemorySize() == 256*256);
    assert(m_gpuInterface->setLocalMemory(oclData, "node_processing",  256*256));
    assert(m_gpuInterface->setLocalMemory(oclData, "synapse_processing",  256*256));
    assert(m_gpuInterface->setLocalMemory(oclData, "output_node_processing",  256*256));
    assert(m_gpuInterface->setLocalMemory(oclData, "output_learn_step",  256*256));

    //==============================================================================================

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "nodes"));
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "nodeBuffers"));

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "inputNodes"));
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "outputInputs"));

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "synapseBuffers"));
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "synapseSections"));

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "bricks"));
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "randomValues"));

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "networkMetaData"));

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "synapseMetaData"));
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "synapseSegmentMeta"));

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "outputMetaData"));
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "outputSegmentMeta"));

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "outputSynapseSections"));
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "outputs"));

    return true;
}

bool
GpuProcessingUnit::updateInput()
{
    return m_gpuInterface->updateBufferOnDevice(oclData, "inputNodes");
}

bool
GpuProcessingUnit::synapse_processing()
{
    assert(m_gpuInterface->run(oclData, "synapse_processing"));
    return true;
}

bool
GpuProcessingUnit::node_processing()
{
    assert(m_gpuInterface->run(oclData, "node_processing"));
    assert(m_gpuInterface->copyFromDevice(oclData, "outputInputs"));

    return true;
}

bool
GpuProcessingUnit::output_node_processing()
{
    assert(m_gpuInterface->run(oclData, "output_node_processing"));
    assert(m_gpuInterface->copyFromDevice(oclData, "outputs"));
    return true;
}

bool
GpuProcessingUnit::output_learn_step()
{
    assert(m_gpuInterface->run(oclData, "output_learn_step"));
    assert(m_gpuInterface->copyFromDevice(oclData, "outputs"));
    return true;
}

bool GpuProcessingUnit::finish()
{
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;
    OutputInput* outputInputs = cluster->outputSegment->inputs;
    for(uint32_t i = 0; i < cluster->outputSegment->segmentMeta->numberOfInputs; i++) {
        outputInputs[i].isNew = 0;
    }
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "outputInputs"));
}

/**
 * @brief run thread to execute gpu-code
 */
void
GpuProcessingUnit::run()
{
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    float timeValue = 0.0f;

    while(!m_abort)
    {
        m_phase1->triggerBarrier();
        m_phase2->triggerBarrier();

        m_phase3->triggerBarrier();
    }
}
