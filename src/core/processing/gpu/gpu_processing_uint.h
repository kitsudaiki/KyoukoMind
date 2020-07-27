/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef GPU_INTERFACE_H
#define GPU_INTERFACE_H

#include <common.h>

#include <libKitsunemimiOpencl/gpu_handler.h>
#include <libKitsunemimiOpencl/gpu_interface.h>

namespace KyoukoMind
{
class Segment;

class GpuProcessingUnit
{
public:
    GpuProcessingUnit();

    bool initializeGpu(Segment &segment,
                       const uint32_t numberOfBricks);
    bool copySynapseTransfersToGpu(Segment &segment);
    bool copyGlobalValuesToGpu();
    bool runOnGpu(const std::string &kernelName);
    bool copyAxonTransfersFromGpu();
    bool closeDevice();

private:
    Kitsunemimi::Opencl::GpuHandler* m_gpuHandler = nullptr;
    Kitsunemimi::Opencl::GpuInterface* m_gpuInterface = nullptr;

    Kitsunemimi::Opencl::OpenClData oclData;
};

}

#endif // GPU_INTERFACE_H
