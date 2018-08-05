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

#include <common.h>
#include <commonThread.h>

namespace KyoukoMind
{
class Cluster;
class ClusterQueue;

class ProcessingUnit : public Kitsune::CommonThread
{

public:
    ProcessingUnit(ClusterQueue* clusterQueue);

    void run();

    virtual void processCluster(Cluster* cluster) = 0;

protected:
    ClusterQueue* m_clusterQueue = nullptr;

    std::vector<Cluster*> m_finishClusterBuffer;

};

}

#endif // PROCESSINGUNIT_H
