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

#include <core/processing/processingThreads/processingUnit.h>
#include <core/structs/kyochanNodes.h>
#include <core/structs/kyochanEdges.h>

namespace KyoukoMind
{
class NextChooser;
class ClusterProcessing;
class NodeClusterProcessing;

class OutgoingMessageBuffer;
class IncomingMessageBuffer;

class CpuProcessingUnit : public ProcessingUnit
{
public:
    CpuProcessingUnit(ClusterQueue *clusterQueue);
    ~CpuProcessingUnit();

    void processCluster(EdgeCluster* cluster);

private:
    NextChooser* m_nextChooser = nullptr;
    ClusterProcessing* m_clusterProcessing = nullptr;
    NodeClusterProcessing* m_nodeProcessing = nullptr;
};

}

#endif // CPUPROCESSINGUNIT_H
