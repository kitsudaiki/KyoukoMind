/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef PROCESSING_UNIT_HANDLER_H
#define PROCESSING_UNIT_HANDLER_H

#include <common.h>

namespace Kitsunemimi {
class Barrier;
namespace Opencl {
class GpuHandler;
}
}

namespace KyoukoMind
{
class CpuProcessingUnit;
class GpuProcessingUnit;

class ProcessingUnitHandler
{
public:
    ProcessingUnitHandler();
    ~ProcessingUnitHandler();

    std::vector<CpuProcessingUnit*> m_cpuProcessingUnits;
    std::vector<GpuProcessingUnit*> m_gpuProcessingUnits;

    bool initProcessingUnits(Kitsunemimi::Barrier* cpuBarrier,
                             Kitsunemimi::Barrier* gpuBarrier,
                             const uint16_t numberOfThreads);
    bool closeAllProcessingUnits();

private:
    Kitsunemimi::Opencl::GpuHandler* m_gpuHandler = nullptr;
};

} // namespace KyoukoMind

#endif // PROCESSING_UNIT_HANDLER_H
