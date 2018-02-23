#ifndef CPUPROCESSINGUNIT_H
#define CPUPROCESSINGUNIT_H

#include <core/processing/processingUnit.h>

namespace KyoChan_Network
{

class CpuProcessingUnit : public ProcessingUnit
{
public:
    CpuProcessingUnit();

    void processCluster(Cluster* cluster);
};

}

#endif // CPUPROCESSINGUNIT_H
