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
    CpuProcessingUnit(ClusterHandler *clusterHandler);
    ~CpuProcessingUnit();

    void processCluster(Cluster* cluster);
};

}

#endif // CPUPROCESSINGUNIT_H
