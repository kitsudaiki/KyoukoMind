/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "gpu_interface.h"

#include <libKitsunemimiOpencl/opencl.h>
#include <gpu_processing.h>
#include <core/processing/internal/objects/transfer_objects.h>
#include <core/processing/internal/objects/node.h>
#include <core/object_handling/segment.h>

namespace KyoukoMind
{

GpuInterface::GpuInterface()
{

}

/**
 * @brief initializeGpu
 * @param ocl
 * @param data
 * @param numberOfBricks
 * @return
 */
bool
GpuInterface::initializeGpu(Segment &segment,
                            const uint32_t numberOfBricks)
{
    const std::string kernelCode(reinterpret_cast<char*>(gpu_processing_cl),
                                 gpu_processing_cl_len);

    // create config-object
    Kitsunemimi::Opencl::OpenClConfig oclConfig;
    oclConfig.kernelCode = kernelCode;
    oclConfig.kernelName = "processing";

    // init gpu-connection
    if(ocl.initDevice(oclConfig) == false) {
        return false;
    }

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
    oclData.buffer[5].data = segment.randomfloatValues.buffer.data;
    oclData.buffer[5].numberOfBytes = segment.randomfloatValues.buffer.bufferPosition;
    oclData.buffer[5].numberOfObjects = segment.randomfloatValues.itemCapacity;

    oclData.buffer[6].data = segment.randomIntValues.buffer.data;
    oclData.buffer[6].numberOfBytes = segment.randomIntValues.buffer.bufferPosition;
    oclData.buffer[6].numberOfObjects = segment.randomIntValues.itemCapacity;

    // fill buffer for global values to map on gpu
    oclData.buffer[7].data = segment.globalValues.buffer.data;
    oclData.buffer[7].numberOfBytes = segment.globalValues.buffer.bufferPosition;
    oclData.buffer[7].numberOfObjects = segment.globalValues.itemCapacity;
    oclData.buffer[7].useHostPtr = true;

    // TODO: replace with a validation to make sure, that the local memory is big enough
    assert(ocl.getLocalMemorySize() == 256*256);
    oclData.localMemorySize = 256*256;

    if(ocl.initCopyToDevice(oclData) == false) {
        return false;
    }

    ocl.updateBufferOnDevice(oclData.buffer[3]);
    //segment.ocl.updateBufferOnDevice(segment.oclData.buffer[4]);

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
GpuInterface::copySynapseTransfersToGpu(Segment &segment)
{
    return ocl.updateBufferOnDevice(oclData.buffer[0],
                                    segment.synapseTransfers.numberOfItems);
}

/**
 * @brief updateNodeOnDevice
 * @param segment
 * @return
 */
bool
GpuInterface::updateNodeOnDevice(const uint32_t nodeId,
                                 const float value)
{
    const uint64_t pos = nodeId * sizeof(Node);

    cl::CommandQueue* queue = &ocl.m_queue;
    const cl_int ret = queue->enqueueWriteBuffer(oclData.buffer[3].clBuffer,
                                                 CL_TRUE,
                                                 pos,
                                                 sizeof(float),
                                                 &value);
    if(ret != CL_SUCCESS) {
        return false;
    }

    return true;
}

/**
 * @brief runOnGpu
 * @param ocl
 * @param data
 * @return
 */
bool
GpuInterface::runOnGpu()
{
    return ocl.run(oclData);
}

/**
 * @brief copyDataFromGpu
 * @param segment
 * @param ocl
 * @param data
 * @return
 */
bool
GpuInterface::copyAxonTransfersFromGpu()
{
    return ocl.copyFromDevice(oclData);
}

/**
 * @brief closeDevice
 * @param ocl
 * @param data
 * @return
 */
bool
GpuInterface::closeDevice()
{
    // because the memory was allocated at another point, it should not be free by the close-process
    // of the device
    oclData.buffer[0].data = nullptr;
    oclData.buffer[1].data = nullptr;
    oclData.buffer[2].data = nullptr;
    oclData.buffer[3].data = nullptr;

    return ocl.closeDevice(oclData);
}

}
