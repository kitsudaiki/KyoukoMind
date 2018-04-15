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

    void processIncomEdge(uint8_t* data,
                          OutgoingMessageBuffer* outgoBuffer);
    void processIncomPendingEdge(uint8_t* data,
                                 OutgoingMessageBuffer* outgoBuffer);
    void processIncomAxonEdge(uint8_t* data,
                              OutgoingMessageBuffer* outgoBuffer);
    void processIncomLerningEdge(uint8_t* data,
                                 uint8_t initSide,
                                 OutgoingMessageBuffer* outgoBuffer);
    void processIncomLerningReplyEdge(uint8_t* data,
                                      uint8_t initSide,
                                      OutgoingMessageBuffer* outgoBuffer);

    bool processNodes();
    bool processAxons();

    void checkPendingClusterEdges();
    uint8_t checkPendingEdges(KyoChanPendingEdge* pendingEdges,
                           const uint8_t numberOfPendingEdges);
    bool addPendingEdges(const KyoChanPendingEdge &newEdge,
                         KyoChanPendingEdge* pendingEdges,
                         const uint8_t numberOfPendingEdges);

    std::vector<uint8_t> m_sideOrder;
    NextChooser* m_nextChooser = nullptr;

    uint8_t m_currentClusterType = UNDEFINED_CLUSTER;
    Cluster* m_currentCluster = nullptr;
    KyoChanAxon* m_axonBlock = nullptr;
    uint32_t m_numberOfAxons = 0;
    KyoChanNode* m_nodeBlock = nullptr;
    uint16_t m_numberOfNodes = 0;
};

}

#endif // CPUPROCESSINGUNIT_H
