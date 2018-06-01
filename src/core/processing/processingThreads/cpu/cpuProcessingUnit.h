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
class EdgeClusterProcessing;
class MessageProcessing;

class EdgeCluster;
class NodeCluster;

class OutgoingMessageBuffer;
class IncomingMessageBuffer;

class CpuProcessingUnit : public ProcessingUnit
{
public:
    CpuProcessingUnit(ClusterQueue *clusterQueue);
    ~CpuProcessingUnit();

    void processCluster(Cluster* cluster);

private:
    bool processMessagesNodeCluster(NodeCluster* cluster);
    bool processMessagesEdgesCluster(EdgeCluster* cluster);
    uint16_t processNodes(NodeCluster *nodeCluster);

    PossibleKyoChanNodes m_activeNodes;

    NextChooser* m_nextChooser = nullptr;
    EdgeClusterProcessing* m_clusterProcessing = nullptr;
    MessageProcessing* m_messageProcessing = nullptr;
};

}

#endif // CPUPROCESSINGUNIT_H
