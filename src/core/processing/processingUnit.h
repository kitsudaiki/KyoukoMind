#ifndef PROCESSINGUNIT_H
#define PROCESSINGUNIT_H

#include <core/processing/commonthread.h>

namespace KyoukoMind
{
class Cluster;
class ClusterHandler;

class ProcessingUnit : public CommonThread
{

public:
    ProcessingUnit();

    void run();

    virtual void processCluster(Cluster* cluster) = 0;
};

}

#endif // PROCESSINGUNIT_H
