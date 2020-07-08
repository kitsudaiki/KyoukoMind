/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "gpu_interface.h"

#include <libKitsunemimiOpencl/opencl.h>
#include <gpu_processing.h>
#include <core/processing/objects/transfer_objects.h>
#include <core/processing/objects/node.h>
#include <core/object_handling/network_segment.h>

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
    segment.oclData.threadsPerWg.x = 256;

    // add empty buffer
    segment.oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer());
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

    // fill buffer for update-edges from gpu to host
    segment.oclData.buffer[2].data = segment.updateEdges.buffer.data;
    segment.oclData.buffer[2].numberOfBytes = segment.updateEdges.buffer.bufferPosition;
    segment.oclData.buffer[2].numberOfObjects = segment.updateEdges.numberOfItems;
    segment.oclData.buffer[2].isOutput = true;

    // fill buffer for nodes to map on gpu
    segment.oclData.buffer[3].data = segment.nodes.buffer.data;
    segment.oclData.buffer[3].numberOfBytes = segment.nodes.buffer.bufferPosition;
    segment.oclData.buffer[3].numberOfObjects = segment.nodes.numberOfItems;

    // fill buffer for synapse-sections to map on gpu
    segment.oclData.buffer[4].data = segment.synapses.buffer.data;
    segment.oclData.buffer[4].numberOfBytes = segment.synapses.buffer.bufferPosition;
    segment.oclData.buffer[4].numberOfObjects = segment.synapses.numberOfItems;

    initRandValues(segment);

    // TODO: replace with a validation to make sure, that the local memory is big enough
    assert(segment.ocl.getLocalMemorySize() == 256*256);
    segment.oclData.localMemorySize = 256*256;

    if(segment.ocl.initCopyToDevice(segment.oclData) == false) {
        return false;
    }

    segment.ocl.updateBufferOnDevice(segment.oclData.buffer[3]);
    //segment.ocl.updateBufferOnDevice(segment.oclData.buffer[4]);

    return true;
}

/**
 * @brief initRandValues
 * @param segment
 * @return
 */
void
initRandValues(NetworkSegment &segment)
{
    segment.oclData.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer(1, sizeof(RandTransfer)));

    RandTransfer* randValues = static_cast<RandTransfer*>(segment.oclData.buffer[5].data);

    float compare = 0.0f;
    for(uint32_t i = 0; i < 999; i++)
    {
        if(i % 3 == 0) {
            compare = 0.0f;
        }

        float tempValue = static_cast<float>(rand()) / 0x7FFFFFFF;
        assert(tempValue <= 1.0f);
        if(tempValue + compare > 1.0f) {
            tempValue = 1.0f - compare;
        }
        compare += tempValue;
        randValues->randWeight[i] = tempValue;
        //std::cout<<randValues->randWeight[i]<<std::endl;
    }

    for(uint32_t i = 0; i < 1024; i++)
    {
        randValues->randPos[i] = static_cast<uint32_t>(rand());
    }
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
    return segment.ocl.updateBufferOnDevice(segment.oclData.buffer[0],
                                            segment.synapseEdgesCounter);
}

/**
 * @brief updateNodeOnDevice
 * @param segment
 * @return
 */
bool
updateNodeOnDevice(NetworkSegment &segment,
                   const uint32_t nodeId,
                   const float value)
{
    const uint64_t pos = nodeId * sizeof(Node);

    cl::CommandQueue* queue = &segment.ocl.m_queue;
    const cl_int ret = queue->enqueueWriteBuffer(segment.oclData.buffer[3].clBuffer,
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
