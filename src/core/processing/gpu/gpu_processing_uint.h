/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef GPU_INTERFACE_H
#define GPU_INTERFACE_H

#include <common.h>
#include <libKitsunemimiCommon/threading/thread.h>

#include <libKitsunemimiOpencl/gpu_handler.h>
#include <libKitsunemimiOpencl/gpu_interface.h>

namespace Kitsunemimi {
class Barrier;
}

namespace KyoukoMind
{
class Segment;

class GpuProcessingUnit
        : public Kitsunemimi::Thread
{
public:
    GpuProcessingUnit(Kitsunemimi::Opencl::GpuInterface* gpuInterface);

    bool initializeGpu(Segment &segment,
                       const uint32_t numberOfBricks);

    void run();

    Kitsunemimi::Barrier* m_phase1 = nullptr;
    Kitsunemimi::Barrier* m_phase2 = nullptr;
    Kitsunemimi::Barrier* m_phase3 = nullptr;

private:
    Kitsunemimi::Opencl::GpuHandler* m_gpuHandler = nullptr;
    Kitsunemimi::Opencl::GpuInterface* m_gpuInterface = nullptr;

    Kitsunemimi::Opencl::OpenClData oclData;

    bool copySynapseTransfersToGpu(Segment &segment);
    bool copyGlobalValuesToGpu();
    bool runOnGpu(const std::string &kernelName);
    bool copyAxonTransfersFromGpu();
    bool closeDevice();
};

}

#endif // GPU_INTERFACE_H
