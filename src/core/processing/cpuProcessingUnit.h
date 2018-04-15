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
#include <core/structs/kyochanNodes.h>
#include <core/structs/kyochanEdges.h>

namespace KyoukoMind
{
class Cluster;
class NodeCluster;
class EdgeCluster;
class EmptyCluster;

class Message;
class DataMessage;
class ReplyMessage;
class NextChooser;

class OutgoingMessageBuffer;
class IncomingMessageBuffer;

class CpuProcessingUnit : public ProcessingUnit
{
public:
    CpuProcessingUnit(ClusterQueue *clusterQueue);
    ~CpuProcessingUnit();

    void processCluster(Cluster* cluster);

private:
    bool processIncomingMessages();

    bool processIncomEdge(uint8_t* data);
    bool processIncomAxonEdge(uint8_t* data);
    bool processIncomLerningEdge(uint8_t* data);
    bool processIncomLerningReplyEdge(uint8_t* data);

    bool processNodes();
    bool processAxons();

    std::vector<uint8_t> m_sideOrder;
    NextChooser* m_nextChooser = nullptr;

    uint8_t m_currentClusterType = UNDEFINED_CLUSTER;
    Cluster* m_currentCluster = nullptr;
};

}

#endif // CPUPROCESSINGUNIT_H
