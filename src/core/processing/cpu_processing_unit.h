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

    void learnNetworkCluster(NetworkCluster* cluster);

    void processNetworkCluster(NetworkCluster* cluster);
};

#endif // CPUPROCESSINGUNIT_H
