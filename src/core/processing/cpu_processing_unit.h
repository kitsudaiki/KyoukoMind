#ifndef CPUPROCESSINGUNIT_H
#define CPUPROCESSINGUNIT_H

#include <common.h>

class NetworkCluster;
class DynamicSegment;
class InputSegment;
class OutputSegment;

class CpuProcessingUnit
{
public:
    CpuProcessingUnit();

    void processNetworkCluster(NetworkCluster* cluster);

private:
    void processDynamicSegment(DynamicSegment* segment);
    void processInputSegment(InputSegment* segment);
    void processOutputSegment(OutputSegment* segment);
};

#endif // CPUPROCESSINGUNIT_H
