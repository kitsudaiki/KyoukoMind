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

#include <core/objects/transfer_objects.h>
#include <core/objects/node.h>

#include <core/objects/segment.h>

#include <libKitsunemimiPersistence/logger/logger.h>


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
GpuProcessingUnit::initializeGpu(Segment &segment,
                                 const uint32_t numberOfBricks)
{
    const std::string processingCode(reinterpret_cast<char*>(synapse_node_processing_cl),
                                     synapse_node_processing_cl_len);

    // init worker-sizes
    oclData.numberOfWg.x = numberOfBricks;
    // only 255 threads per group, instead of 256, because 1/256 of the local memory is used for
    // the global-values object
    oclData.threadsPerWg.x = 255;

    // fill buffer for synapses from host to gpu
    oclData.addBuffer("synapse-transfers",
                      segment.synapseTransfers.itemCapacity,
                      segment.synapseTransfers.itemSize,
                      false,
                      true,
                      segment.synapseTransfers.buffer.data);

    // fill buffer for axons from gpu to host
    oclData.addBuffer("axons",
                      segment.axonTransfers.itemCapacity,
                      segment.axonTransfers.itemSize,
                      true,
                      false,
                      segment.axonTransfers.buffer.data);

    // fill buffer for update-transfers from gpu to host
    oclData.addBuffer("update-transfers",
                      segment.updateTransfers.itemCapacity,
                      segment.updateTransfers.itemSize,
                      true,
                      false,
                      segment.updateTransfers.buffer.data);

    // fill buffer for nodes to map on gpu
    oclData.addBuffer("nodes",
                      segment.nodes.itemCapacity,
                      segment.nodes.itemSize,
                      false,
                      false,
                      segment.nodes.buffer.data);

    // fill buffer for synapse-sections to map on gpu
    oclData.addBuffer("synapses",
                      segment.synapses.itemCapacity,
                      segment.synapses.itemSize,
                      false,
                      false,
                      segment.synapses.buffer.data);

    // fill buffer for random values to map on gpu
    oclData.addBuffer("random-values",
                      segment.randomIntValues.itemCapacity,
                      segment.randomIntValues.itemSize,
                      false,
                      false,
                      segment.randomIntValues.buffer.data);

    // fill buffer for global values to map on gpu
    oclData.addBuffer("global-values",
                      segment.globalValues.itemCapacity,
                      segment.globalValues.itemSize,
                      false,
                      false,
                      segment.globalValues.buffer.data);

    assert(m_gpuInterface->initCopyToDevice(oclData));

    // init kernel
    assert(m_gpuInterface->addKernel(oclData, "synapse_processing",  processingCode));
    assert(m_gpuInterface->addKernel(oclData, "sum_nodes",  processingCode));
    assert(m_gpuInterface->addKernel(oclData, "node_processing",  processingCode));
    assert(m_gpuInterface->addKernel(oclData, "updating",  processingCode));
    assert(m_gpuInterface->addKernel(oclData, "hardening",  processingCode));

    // bind buffer for synapse_processing kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "synapse-transfers"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "nodes"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "synapses"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "random-values"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "synapse_processing", "global-values"));

    // bind buffer for sum_nodes kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "sum_nodes", "nodes"));

    // bind buffer for node_processing kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "node_processing", "axons"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "node_processing", "nodes"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "node_processing", "global-values"));

    // bind buffer for updating kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "updating", "update-transfers"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "updating", "nodes"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "updating", "synapses"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "updating", "global-values"));

    // bind buffer for hardening kernel
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "hardening", "synapses"));
    assert(m_gpuInterface->bindKernelToBuffer(oclData, "hardening", "global-values"));

    // init local memory for the kernels
    assert(m_gpuInterface->getLocalMemorySize() == 256*256);
    assert(m_gpuInterface->setLocalMemory(oclData, "synapse_processing",  256*256));
    assert(m_gpuInterface->setLocalMemory(oclData, "sum_nodes",  256*256));
    assert(m_gpuInterface->setLocalMemory(oclData, "node_processing",  256*256));
    assert(m_gpuInterface->setLocalMemory(oclData, "updating",  256*256));
    assert(m_gpuInterface->setLocalMemory(oclData, "hardening",  256*256));

    return true;
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

    Segment* segment = KyoukoRoot::m_segment;

    while(!m_abort)
    {
        m_phase1->triggerBarrier();
        m_phase2->triggerBarrier();

        // copy transfer-edges to gpu
        start = std::chrono::system_clock::now();
        copySynapseTransfersToGpu(*segment);
        copyGlobalValuesToGpu();
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        KyoukoRoot::monitoringMetaMessage.copyToGpu = timeValue;

        start = std::chrono::system_clock::now();
        runOnGpu("synapse_processing");
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        KyoukoRoot::monitoringMetaMessage.gpuSynapse = timeValue;

        start = std::chrono::system_clock::now();
        runOnGpu("sum_nodes");
        //end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        //KyoukoRoot::monitoringMetaMessage.gpuSynapse = timeValue;

        //start = std::chrono::system_clock::now();
        runOnGpu("node_processing");
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        KyoukoRoot::monitoringMetaMessage.gpuNode = timeValue;

        // run process on gpu
        start = std::chrono::system_clock::now();
        runOnGpu("hardening");
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        //KyoukoRoot::monitoringMetaMessage.gpuUpdate = timeValue;

        // run process on gpu
        start = std::chrono::system_clock::now();
        runOnGpu("updating");
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        KyoukoRoot::monitoringMetaMessage.gpuUpdate = timeValue;

        // copy result from gpu to host
        start = std::chrono::system_clock::now();
        copyAxonTransfersFromGpu();
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        KyoukoRoot::monitoringMetaMessage.copyFromGpu = timeValue;

        start = std::chrono::system_clock::now();
        segment->synapseTransfers.deleteAll();
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        KyoukoRoot::monitoringMetaMessage.cleanup = timeValue;
        m_phase3->triggerBarrier();
    }
}

/**
 * @brief copy synapse transfers from host to gpu
 *
 * @param segment segment with the data to copy
 *
 * @return true, if successfull, else false
 */
bool
GpuProcessingUnit::copySynapseTransfersToGpu(Segment &segment)
{
    return m_gpuInterface->updateBufferOnDevice(oclData,
                                                "synapse_processing",
                                                "synapse-transfers",
                                                segment.synapseTransfers.numberOfItems);
}

/**
 * @brief copy global values from host to gpu
 *
 * @return true, if successfull, else false
 */
bool
GpuProcessingUnit::copyGlobalValuesToGpu()
{
    return m_gpuInterface->updateBufferOnDevice(oclData, "synapse_processing", "global-values", 1);
}

/**
 * @brief run kernel
 *
 * @param kernelName name of the kernel
 *
 * @return true, if successfull, else false
 */
bool
GpuProcessingUnit::runOnGpu(const std::string &kernelName)
{
    return m_gpuInterface->run(oclData, kernelName);
}

/**
 * @brief copy axons from gpu to host
 *
 * @return true, if successfull, else false
 */
bool
GpuProcessingUnit::copyAxonTransfersFromGpu()
{
    return m_gpuInterface->copyFromDevice(oclData);
}

/**
 * @brief close connection to gpu
 *
 * @return true, if successfull, else false
 */
bool
GpuProcessingUnit::closeDevice()
{
    return m_gpuInterface->closeDevice(oclData);
}
