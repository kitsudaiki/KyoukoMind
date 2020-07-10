/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef GPU_INTERFACE_H
#define GPU_INTERFACE_H

#include <common.h>

#include <libKitsunemimiOpencl/opencl.h>

namespace KyoukoMind
{
class NetworkSegment;

class GpuInterface
{
public:
    GpuInterface();

    bool initializeGpu(NetworkSegment &segment,
                       const uint32_t numberOfBricks);
    bool copyEdgesToGpu(NetworkSegment &segment);
    bool updateNodeOnDevice(const uint32_t nodeId,
                            const float value);
    bool runOnGpu();
    bool copyAxonsFromGpu();
    bool closeDevice();

private:
    Kitsunemimi::Opencl::Opencl ocl;
    Kitsunemimi::Opencl::OpenClData oclData;

};

}

#endif // GPU_INTERFACE_H
