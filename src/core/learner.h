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

class Learner
{
public:
    Learner();

    uint32_t learnStep(uint32_t label);
    void executeStep(const uint32_t runs);

    Batch batchs[10];

    float buffer[2400];
private:
    Kitsunemimi::Opencl::GpuHandler* m_gpuHandler = nullptr;
    GpuProcessingUnit* m_gpu = nullptr;

    void finishStep();
    uint32_t checkOutput(OutputSegmentMeta *segmentMeta, Output *outputs);
};

#endif // LERNER_H
