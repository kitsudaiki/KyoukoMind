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

    void processCluster(EdgeCluster* cluster);

private:
    bool processMessagesEdges(EdgeCluster* cluster);
    uint16_t processNodes(NodeCluster *nodeCluster);

    std::vector<uint8_t> m_sideOrder;
    NextChooser* m_nextChooser = nullptr;
    ClusterProcessing* m_clusterProcessing = nullptr;
    MessageProcessing* m_messageProcessing = nullptr;
};

}

#endif // CPUPROCESSINGUNIT_H
