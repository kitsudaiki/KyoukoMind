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

#include <core/processing/processingThreads/commonThread.h>

namespace KyoukoMind
{
class EdgeCluster;
class ClusterQueue;

class ProcessingUnit : public CommonThread
{

public:
    ProcessingUnit(ClusterQueue* clusterQueue);

    void run();

    virtual void processCluster(EdgeCluster* cluster) = 0;

protected:
    ClusterQueue* m_clusterQueue = nullptr;

    std::vector<EdgeCluster*> m_finishClusterBuffer;

};

}

#endif // PROCESSINGUNIT_H
