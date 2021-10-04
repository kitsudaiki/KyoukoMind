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

#include <core/objects/node.h>
#include <core/orchestration/segments/dynamic_segment.h>
#include <core/orchestration/network_cluster.h>

#include <libKitsunemimiCommon/logger.h>

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
    const uint64_t availableLocalMem = m_gpuInterface->getLocalMemorySize();

    // init worker-sizes
    oclData.numberOfWg.x = 1;
    // only 127 threads per group, instead of 128, because 1/128 of the local memory is used for
    // the meta-data
    oclData.threadsPerWg.x = (availableLocalMem / 512) - 1;

    //==============================================================================================

    // fill buffer for nodes to map on gpu
    /*SegmentHeader* segmentHeader = cluster->synapseSegment->segmentHeader;

    oclData.addBuffer("segment_data",
                      1,
                      segmentHeader->staticDataSize,
                      false,
                      cluster->synapseSegment->segmentData.staticData);

    oclData.addBuffer("inputs",
                      segmentHeader->inputs.count,
                      sizeof(InputNode),
                      false,
                      cluster->synapseSegment->inputs);
    oclData.addBuffer("outputs",
                      segmentHeader->outputs.count,
                      sizeof(OutputNode),
                      false,
                      cluster->synapseSegment->outputs);

    oclData.addBuffer("randomValues",
                      NUMBER_OF_RAND_VALUES,
                      sizeof(uint32_t),
                      false,
                      cluster->randomValues);

    //==============================================================================================

    assert(m_gpuInterface->initCopyToDevice(oclData));

    //==============================================================================================

    // init kernel
    assert(m_gpuInterface->addKernel(oclData, "learn",  processingCode));
    assert(m_gpuInterface->addKernel(oclData, "execute",  processingCode));

    // bind buffer for learn kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "learn", "segment_data"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "learn", "inputs"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "learn", "outputs"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "learn", "randomValues"));

    // bind buffer for execute kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "execute", "segment_data"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "execute", "inputs"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "execute", "outputs"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "execute", "randomValues"));


    //==============================================================================================

    // init local memory for the kernels
    assert(m_gpuInterface->setLocalMemory(oclData, "learn",  availableLocalMem));
    assert(m_gpuInterface->setLocalMemory(oclData, "execute",  availableLocalMem));

    //==============================================================================================

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "randomValues"));

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "inputs"));
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "outputs"));

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "segment_data"));*/

    return true;
}

bool
GpuProcessingUnit::learn()
{
    std::cout<<"learn"<<std::endl;
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "inputs"));
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "outputs"));
    assert(m_gpuInterface->run(oclData, "learn"));

    return true;
}

bool
GpuProcessingUnit::execute()
{
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "inputs"));
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "outputs"));
    assert(m_gpuInterface->run(oclData, "execute"));
    assert(m_gpuInterface->copyFromDevice(oclData, "outputs"));

    return true;
}
