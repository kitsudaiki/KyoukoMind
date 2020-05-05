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
              Kitsunemimi::Opencl::Opencl &ocl,
              Kitsunemimi::Opencl::OpenClData &oclData,
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
    oclData.threadsPerWg.x = ocl.getMaxWorkItemSize().x;

    // add empty buffer
    oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());
    oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());
    oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());
    oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());

    // fill buffer for edges from host to gpu
    oclData.buffer[0].data = segment.synapseEdges.buffer.data;
    oclData.buffer[0].numberOfBytes = segment.synapseEdges.buffer.bufferPosition;
    oclData.buffer[0].numberOfObjects = segment.synapseEdges.numberOfItems;
    oclData.buffer[0].useHostPtr = true;

    // fill buffer for axons from gpu to host
    oclData.buffer[1].data = segment.axonEdges.buffer.data;
    oclData.buffer[1].numberOfBytes = segment.axonEdges.buffer.bufferPosition;
    oclData.buffer[1].numberOfObjects = segment.axonEdges.numberOfItems;
    oclData.buffer[1].isOutput = true;

    // fill buffer for nodes to map on gpu
    oclData.buffer[2].data = segment.nodes.buffer.data;
    oclData.buffer[2].numberOfBytes = segment.synapses.buffer.bufferPosition;
    oclData.buffer[2].numberOfObjects = segment.synapses.numberOfItems;

    // fill buffer for synapse-sections to map on gpu
    oclData.buffer[3].data = segment.synapses.buffer.data;
    oclData.buffer[3].numberOfBytes = segment.synapses.buffer.bufferPosition;
    oclData.buffer[3].numberOfObjects = segment.synapses.numberOfItems;

    if(ocl.initCopyToDevice(oclData) == false) {
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
copyEdgesToGpu(Kitsunemimi::Opencl::Opencl &ocl,
               Kitsunemimi::Opencl::OpenClData &data)
{
    return ocl.updateBufferOnDevice(data.buffer[0]);
}

/**
 * @brief runOnGpu
 * @param ocl
 * @param data
 * @return
 */
bool
runOnGpu(Kitsunemimi::Opencl::Opencl &ocl,
         Kitsunemimi::Opencl::OpenClData &data)
{
    return ocl.run(data);
}

/**
 * @brief copyDataFromGpu
 * @param segment
 * @param ocl
 * @param data
 * @return
 */
bool
copyAxonsFromGpu(Kitsunemimi::Opencl::Opencl &ocl,
                 Kitsunemimi::Opencl::OpenClData &data)
{
    return ocl.copyFromDevice(data);
}

/**
 * @brief closeDevice
 * @param ocl
 * @param data
 * @return
 */
bool
closeDevice(Kitsunemimi::Opencl::Opencl &ocl,
            Kitsunemimi::Opencl::OpenClData &data)
{
    // because the memory was allocated at another point, it should not be free by the close-process
    // of the device
    data.buffer[0].data = nullptr;
    data.buffer[1].data = nullptr;
    data.buffer[2].data = nullptr;
    data.buffer[3].data = nullptr;

    return ocl.closeDevice(data);
}

}

#endif // GPU_INTERFACE_H
