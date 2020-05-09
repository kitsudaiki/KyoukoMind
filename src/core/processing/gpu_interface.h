#ifndef GPU_INTERFACE_H
#define GPU_INTERFACE_H

#include <common.h>

#include <core/objects/container_definitions.h>
#include <core/objects/synapses.h>
#include <core/objects/network_segment.h>

#include <libKitsunemimiOpencl/opencl.h>
#include <gpu_processing.h>

namespace KyoukoMind
{

/**
 * @brief initializeGpu
 * @param ocl
 * @param data
 * @param numberOfBricks
 * @return
 */
bool
initializeGpu(NetworkSegment &segment,
              const uint32_t numberOfBricks)
{
    const std::string kernelCode(reinterpret_cast<char*>(gpu_processing_cl),
                                 gpu_processing_cl_len);

    // create config-object
    Kitsunemimi::Opencl::OpenClConfig oclConfig;
    oclConfig.kernelCode = kernelCode;
    oclConfig.kernelName = "processing";

    // init gpu-connection
    if(segment.ocl.initDevice(oclConfig) == false) {
        return false;
    }

    // init worker-sizes
    segment.oclData.numberOfWg.x = numberOfBricks;
    segment.oclData.threadsPerWg.x = segment.ocl.getMaxWorkItemSize().x;

    // add empty buffer
    segment.oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());
    segment.oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());
    segment.oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());
    segment.oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());

    // fill buffer for edges from host to gpu
    segment.oclData.buffer[0].data = segment.synapseEdges.buffer.data;
    segment.oclData.buffer[0].numberOfBytes = segment.synapseEdges.buffer.bufferPosition;
    segment.oclData.buffer[0].numberOfObjects = segment.synapseEdges.numberOfItems;
    segment.oclData.buffer[0].useHostPtr = true;

    // fill buffer for axons from gpu to host
    segment.oclData.buffer[1].data = segment.axonEdges.buffer.data;
    segment.oclData.buffer[1].numberOfBytes = segment.axonEdges.buffer.bufferPosition;
    segment.oclData.buffer[1].numberOfObjects = segment.axonEdges.numberOfItems;
    segment.oclData.buffer[1].isOutput = true;

    // fill buffer for nodes to map on gpu
    segment.oclData.buffer[2].data = segment.nodes.buffer.data;
    segment.oclData.buffer[2].numberOfBytes = segment.synapses.buffer.bufferPosition;
    segment.oclData.buffer[2].numberOfObjects = segment.synapses.numberOfItems;

    // fill buffer for synapse-sections to map on gpu
    segment.oclData.buffer[3].data = segment.synapses.buffer.data;
    segment.oclData.buffer[3].numberOfBytes = segment.synapses.buffer.bufferPosition;
    segment.oclData.buffer[3].numberOfObjects = segment.synapses.numberOfItems;

    if(segment.ocl.initCopyToDevice(segment.oclData) == false) {
        return false;
    }

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
copyEdgesToGpu(NetworkSegment &segment)
{
    return segment.ocl.updateBufferOnDevice(segment.oclData.buffer[0]);
}

/**
 * @brief runOnGpu
 * @param ocl
 * @param data
 * @return
 */
bool
runOnGpu(NetworkSegment &segment)
{
    return segment.ocl.run(segment.oclData);
}

/**
 * @brief copyDataFromGpu
 * @param segment
 * @param ocl
 * @param data
 * @return
 */
bool
copyAxonsFromGpu(NetworkSegment &segment)
{
    return segment.ocl.copyFromDevice(segment.oclData);
}

/**
 * @brief closeDevice
 * @param ocl
 * @param data
 * @return
 */
bool
closeDevice(NetworkSegment &segment)
{
    // because the memory was allocated at another point, it should not be free by the close-process
    // of the device
    segment.oclData.buffer[0].data = nullptr;
    segment.oclData.buffer[1].data = nullptr;
    segment.oclData.buffer[2].data = nullptr;
    segment.oclData.buffer[3].data = nullptr;

    return segment.ocl.closeDevice(segment.oclData);
}

}

#endif // GPU_INTERFACE_H
