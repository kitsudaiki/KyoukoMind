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
    ProcessingUnit();

    void run();

    virtual void processCluster(Cluster* cluster) = 0;
};

}

#endif // PROCESSINGUNIT_H
