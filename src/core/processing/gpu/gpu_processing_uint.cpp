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
    oclData.numberOfWg.x = 1;
    // only 127 threads per group, instead of 128, because 1/128 of the local memory is used for
    // the meta-data
    oclData.threadsPerWg.x = 127;

    //==============================================================================================

    // fill buffer for nodes to map on gpu

    oclData.addBuffer("networkMetaData",    1,                     sizeof(Kitsunemimi::Ai::NetworkMetaData), true,  &cluster->networkMetaData);
    oclData.addBuffer("randomValues",       NUMBER_OF_RAND_VALUES, sizeof(uint32_t),                         false, cluster->randomValues);

    SegmentHeader* segmentHeader = cluster->synapseSegment->segmentHeader;
    oclData.addBuffer("segment_persistent", 1,  segmentHeader->segmentPersistentBufferSize, false,  cluster->synapseSegment->persistenBuffer.data);
    oclData.addBuffer("segment_ephemeral",  1,  segmentHeader->segmentEphemeralBufferSize,  false,  cluster->synapseSegment->ephemeralBuffer.data);

    oclData.addBuffer("inputs",  segmentHeader->inputs.count,  sizeof(InputNode),  true,  cluster->synapseSegment->inputs);
    oclData.addBuffer("outputs", segmentHeader->outputs.count, sizeof(OutputNode), false, cluster->synapseSegment->outputs);

    //==============================================================================================

    assert(m_gpuInterface->initCopyToDevice(oclData));

    //==============================================================================================

    // init kernel
    assert(m_gpuInterface->addKernel(oclData, "processSegmentInput",  processingCode));
    assert(m_gpuInterface->addKernel(oclData, "processSegmentOutput",  processingCode));
    assert(m_gpuInterface->addKernel(oclData, "rewightSegment",  processingCode));
    assert(m_gpuInterface->addKernel(oclData, "hardenSegment",  processingCode));
    assert(m_gpuInterface->addKernel(oclData, "prcessSegmentNodes",  processingCode));

    // bind buffer for processSegmentInput kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "processSegmentInput", "inputs"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "processSegmentInput", "segment_persistent"));

    // bind buffer for processSegmentOutput kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "processSegmentOutput", "outputs"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "processSegmentOutput", "segment_persistent"));

    // bind buffer for rewightSegment kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "rewightSegment", "segment_persistent"));

    // bind buffer for hardenSegment kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "hardenSegment", "synapseBuffers"));

    // bind buffer for prcessSegmentNodes kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "prcessSegmentNodes", "synapseBuffers"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "prcessSegmentNodes", "segment_ephemeral"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "prcessSegmentNodes", "randomValues"));


    //==============================================================================================

    // init local memory for the kernels
    assert(m_gpuInterface->getLocalMemorySize() == 256*256);
    assert(m_gpuInterface->setLocalMemory(oclData, "processSegmentInput",  256*256));
    assert(m_gpuInterface->setLocalMemory(oclData, "processSegmentOutput",  256*256));
    assert(m_gpuInterface->setLocalMemory(oclData, "rewightSegment",  256*256));
    assert(m_gpuInterface->setLocalMemory(oclData, "hardenSegment",  256*256));
    assert(m_gpuInterface->setLocalMemory(oclData, "prcessSegmentNodes",  256*256));

    //==============================================================================================

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "networkMetaData"));
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "randomValues"));

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "inputs"));
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "outputs"));

    assert(m_gpuInterface->updateBufferOnDevice(oclData, "segment_persistent"));
    assert(m_gpuInterface->updateBufferOnDevice(oclData, "segment_ephemeral"));

    return true;
}

bool
GpuProcessingUnit::learn()
{
    assert(m_gpuInterface->run(oclData, "processSegmentInput"));
    assert(m_gpuInterface->run(oclData, "prcessSegmentNodes"));
    assert(m_gpuInterface->run(oclData, "processSegmentOutput"));
    assert(m_gpuInterface->run(oclData, "rewightSegment"));
    assert(m_gpuInterface->run(oclData, "hardenSegment"));
}

bool
GpuProcessingUnit::execute()
{
    assert(m_gpuInterface->run(oclData, "processSegmentInput"));
    assert(m_gpuInterface->run(oclData, "prcessSegmentNodes"));
    assert(m_gpuInterface->run(oclData, "processSegmentOutput"));
    assert(m_gpuInterface->copyFromDevice(oclData, "outputs"));
}
