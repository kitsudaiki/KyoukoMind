#ifndef PROCESSINGUNIT_H
#define PROCESSINGUNIT_H

namespace KyoukoMind
{
class Cluster;
class ClusterHandler;

class ProcessingUnit
{

public:
    ProcessingUnit();

    virtual void processCluster(Cluster* cluster) = 0;
};

}

#endif // PROCESSINGUNIT_H
