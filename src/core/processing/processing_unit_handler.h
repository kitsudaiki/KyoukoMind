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

class CpuProcessingUnit;
class GpuProcessingUnit;

class ProcessingUnitHandler
{
public:
    ProcessingUnitHandler();
    ~ProcessingUnitHandler();

    std::vector<CpuProcessingUnit*> m_cpuProcessingUnits;
    std::vector<GpuProcessingUnit*> m_gpuProcessingUnits;

    bool initProcessingUnits(Kitsunemimi::Barrier* phase1,
                             Kitsunemimi::Barrier* phase2,
                             Kitsunemimi::Barrier* phase3,
                             const uint16_t numberOfThreads);
    bool closeAllProcessingUnits();

private:
    Kitsunemimi::Opencl::GpuHandler* m_gpuHandler = nullptr;
};

#endif // PROCESSING_UNIT_HANDLER_H
