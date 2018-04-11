/**
 *  @file    processingUnit.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef PROCESSINGUNIT_H
#define PROCESSINGUNIT_H

#include <core/processing/commonThread.h>

namespace KyoukoMind
{
class Cluster;
class ClusterHandler;

class ProcessingUnit : public CommonThread
{

public:
    ProcessingUnit(ClusterHandler* clusterHandler);

    void run();

    virtual void processCluster(Cluster* cluster) = 0;

protected:
    ClusterHandler* m_clusterHandler = nullptr;
};

}

#endif // PROCESSINGUNIT_H
