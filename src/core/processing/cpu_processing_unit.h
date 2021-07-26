#ifndef CPUPROCESSINGUNIT_H
#define CPUPROCESSINGUNIT_H

#include <common.h>

class CpuProcessingUnit
{
public:
    CpuProcessingUnit();

    bool learn();
    bool execute();

private:
    void executeStep();
    void reductionLearning();

    uint32_t reductionCounter = 0;
};

#endif // CPUPROCESSINGUNIT_H
