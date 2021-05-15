#ifndef LERNER_H
#define LERNER_H

#include <common.h>

struct OutputSegmentMeta;
struct Output;

struct Batch {
    float buffer[2400];
    uint32_t counter = 0;
};

namespace Kitsunemimi {
namespace Opencl {
class GpuHandler;
}
}
class GpuProcessingUnit;

class StaticProcessing
{
public:
    StaticProcessing();

    bool learnStep();
    void executeStep(const uint32_t runs);

    Batch batchs[10];

    float buffer[2400];
private:
    Kitsunemimi::Opencl::GpuHandler* m_gpuHandler = nullptr;
    GpuProcessingUnit* m_gpu = nullptr;

    uint32_t checkOutput(OutputSegmentMeta *segmentMeta, Output *outputs);

    bool learnPhase1();
    bool learnPhase2();
};

#endif // LERNER_H
