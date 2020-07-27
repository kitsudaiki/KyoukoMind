/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "gpu_processing_uint.h"

#include <libKitsunemimiOpencl/gpu_interface.h>
#include <gpu_processing.h>
#include <core/processing/objects/transfer_objects.h>
#include <core/processing/objects/node.h>
#include <core/object_handling/segment.h>

namespace KyoukoMind
{

GpuProcessingUnit::GpuProcessingUnit()
{
    m_gpuHandler = new Kitsunemimi::Opencl::GpuHandler();
    assert(m_gpuHandler->m_interfaces.size() > 0);
    m_gpuInterface = m_gpuHandler->m_interfaces.at(0);
}

/**
 * @brief initializeGpu
 * @param ocl
 * @param data
 * @param numberOfBricks
 * @return
 */
bool
GpuProcessingUnit::initializeGpu(Segment &segment,
                            const uint32_t numberOfBricks)
{
    const std::string processingCode(reinterpret_cast<char*>(gpu_processing_cl),
                                     gpu_processing_cl_len);

    // init worker-sizes
    oclData.numberOfWg.x = numberOfBricks;
    oclData.threadsPerWg.x = 256;

    // add empty buffer
    oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());
    oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());
    oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());
    oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());
    oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());
    oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());
    oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());

    // fill buffer for edges from host to gpu
    oclData.buffer[0].data = segment.synapseTransfers.buffer.data;
    oclData.buffer[0].numberOfBytes = segment.synapseTransfers.buffer.bufferPosition;
    oclData.buffer[0].numberOfObjects = segment.synapseTransfers.itemCapacity;
    oclData.buffer[0].useHostPtr = true;

    // fill buffer for axons from gpu to host
    oclData.buffer[1].data = segment.axonTransfers.buffer.data;
    oclData.buffer[1].numberOfBytes = segment.axonTransfers.buffer.bufferPosition;
    oclData.buffer[1].numberOfObjects = segment.axonTransfers.itemCapacity;
    oclData.buffer[1].isOutput = true;

    // fill buffer for update-edges from gpu to host
    oclData.buffer[2].data = segment.updateTransfers.buffer.data;
    oclData.buffer[2].numberOfBytes = segment.updateTransfers.buffer.bufferPosition;
    oclData.buffer[2].numberOfObjects = segment.updateTransfers.itemCapacity;
    oclData.buffer[2].isOutput = true;

    // fill buffer for nodes to map on gpu
    oclData.buffer[3].data = segment.nodes.buffer.data;
    oclData.buffer[3].numberOfBytes = segment.nodes.buffer.bufferPosition;
    oclData.buffer[3].numberOfObjects = segment.nodes.itemCapacity;

    // fill buffer for synapse-sections to map on gpu
    oclData.buffer[4].data = segment.synapses.buffer.data;
    oclData.buffer[4].numberOfBytes = segment.synapses.buffer.bufferPosition;
    oclData.buffer[4].numberOfObjects = segment.synapses.itemCapacity;

    // fill buffer for random values to map on gpu
    oclData.buffer[5].data = segment.randomIntValues.buffer.data;
    oclData.buffer[5].numberOfBytes = segment.randomIntValues.buffer.bufferPosition;
    oclData.buffer[5].numberOfObjects = segment.randomIntValues.itemCapacity;

    // fill buffer for global values to map on gpu
    oclData.buffer[6].data = segment.globalValues.buffer.data;
    oclData.buffer[6].numberOfBytes = segment.globalValues.buffer.bufferPosition;
    oclData.buffer[6].numberOfObjects = segment.globalValues.itemCapacity;

    assert(m_gpuInterface->initCopyToDevice(oclData));

    assert(m_gpuInterface->addKernel("synapse_processing",  processingCode));
    assert(m_gpuInterface->addKernel("node_processing",  processingCode));
    assert(m_gpuInterface->addKernel("updating",  processingCode));

    assert(m_gpuInterface->bindKernelToBuffer("synapse_processing", 0, oclData));
    assert(m_gpuInterface->bindKernelToBuffer("synapse_processing", 3, oclData));
    assert(m_gpuInterface->bindKernelToBuffer("synapse_processing", 4, oclData));
    assert(m_gpuInterface->bindKernelToBuffer("synapse_processing", 5, oclData));
    assert(m_gpuInterface->bindKernelToBuffer("synapse_processing", 6, oclData));

    assert(m_gpuInterface->bindKernelToBuffer("node_processing", 1, oclData));
    assert(m_gpuInterface->bindKernelToBuffer("node_processing", 3, oclData));
    assert(m_gpuInterface->bindKernelToBuffer("node_processing", 6, oclData));

    assert(m_gpuInterface->bindKernelToBuffer("updating", 2, oclData));
    assert(m_gpuInterface->bindKernelToBuffer("updating", 4, oclData));

    assert(m_gpuInterface->getLocalMemorySize() == 256*256);
    assert(m_gpuInterface->setLocalMemory("synapse_processing",  256*256));
    assert(m_gpuInterface->setLocalMemory("node_processing",  256*256));
    assert(m_gpuInterface->setLocalMemory("updating",  256*256));


    return true;
}

/**
 * @brief copyDataToGpu
 * @param segment
 * @param ocl
 * @param data
 * @return
 */
bool
GpuProcessingUnit::copySynapseTransfersToGpu(Segment &segment)
{
    return m_gpuInterface->updateBufferOnDevice("synapse_processing",
                                                0,
                                                segment.synapseTransfers.numberOfItems);
}

/**
 * @brief GpuInterface::copyGlobalValuesToGpu
 * @param segment
 * @return
 */
bool
GpuProcessingUnit::copyGlobalValuesToGpu()
{
    return m_gpuInterface->updateBufferOnDevice("synapse_processing",
                                                4,
                                                1);
}

/**
 * @brief runOnGpu
 * @param ocl
 * @param data
 * @return
 */
bool
GpuProcessingUnit::runOnGpu(const std::string &kernelName)
{
    return m_gpuInterface->run(oclData, kernelName);
}

/**
 * @brief copyDataFromGpu
 * @param segment
 * @param ocl
 * @param data
 * @return
 */
bool
GpuProcessingUnit::copyAxonTransfersFromGpu()
{
    return m_gpuInterface->copyFromDevice(oclData);
}

/**
 * @brief closeDevice
 * @param ocl
 * @param data
 * @return
 */
bool
GpuProcessingUnit::closeDevice()
{
    // because the memory was allocated at another point, it should not be free by the close-process
    // of the device
    oclData.buffer[0].data = nullptr;
    oclData.buffer[1].data = nullptr;
    oclData.buffer[2].data = nullptr;
    oclData.buffer[3].data = nullptr;

    return m_gpuInterface->closeDevice(oclData);
}

}
