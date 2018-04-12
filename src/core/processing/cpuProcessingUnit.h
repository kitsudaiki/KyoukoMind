/**
 *  @file    cpuProcessingUnit.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef CPUPROCESSINGUNIT_H
#define CPUPROCESSINGUNIT_H

#include <core/processing/processingUnit.h>

namespace KyoukoMind
{

class CpuProcessingUnit : public ProcessingUnit
{
public:
    CpuProcessingUnit(ClusterQueue *clusterQueue);
    ~CpuProcessingUnit();

    void processCluster(Cluster* cluster);

private:
    void processEmptyCluster(Cluster* cluster);
    void processEdgeCluster(Cluster* cluster);
    void processNodeCluster(Cluster* cluster);
};

}

#endif // CPUPROCESSINGUNIT_H
