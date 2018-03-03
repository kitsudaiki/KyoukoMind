#ifndef CPUPROCESSINGUNIT_H
#define CPUPROCESSINGUNIT_H

#include <core/processing/processingUnit.h>

namespace KyoukoMind
{

class CpuProcessingUnit : public ProcessingUnit
{
public:
    CpuProcessingUnit();

    void processCluster(Cluster* cluster);
};

}

#endif // CPUPROCESSINGUNIT_H
