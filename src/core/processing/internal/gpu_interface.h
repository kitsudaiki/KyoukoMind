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
class Segment;

class GpuInterface
{
public:
    GpuInterface();

    bool initializeGpu(Segment &segment,
                       const uint32_t numberOfBricks);
    bool copySynapseTransfersToGpu(Segment &segment);
    bool copyGlobalValuesToGpu();
    bool updateNodeOnDevice(const uint32_t nodeId,
                            const float value);
    bool runOnGpu(const std::string &kernelName);
    bool copyAxonTransfersFromGpu();
    bool closeDevice();

private:
    Kitsunemimi::Opencl::Opencl ocl;

    Kitsunemimi::Opencl::OpenClConfig oclProcessingConfig;
    Kitsunemimi::Opencl::OpenClConfig oclUpdateConfig;

    Kitsunemimi::Opencl::OpenClData oclData;

};

}

#endif // GPU_INTERFACE_H
