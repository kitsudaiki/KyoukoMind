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
class LearningMessage;
class LearningReplyMessage;
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
    bool processEmptyCluster(Cluster* cluster);
    bool processEdgeCluster(Cluster* cluster);
    bool processNodeCluster(Cluster* cluster);

    void processIncomingMessages(Cluster *cluster,
                                 KyoChanNode *nodeBlock,
                                 const uint16_t numberOfNodes);

    void processDataMessage(DataMessage *message,
                            const ClusterID targetId,
                            const uint8_t inputSide,
                            KyoChanNode *nodeBlock,
                            const uint16_t numberOfNodes,
                            OutgoingMessageBuffer *outgoBuffer);
    void processLearningMessage(LearningMessage *message,
                                const ClusterID targetId,
                                const uint8_t inputSide,
                                KyoChanNode *nodeBlock,
                                const uint16_t numberOfNodes,
                                OutgoingMessageBuffer *outgoBuffer);
    void processLearningRyplyMessage(LearningReplyMessage *message,
                                     const ClusterID targetId,
                                     const uint8_t inputSide,
                                     KyoChanNode *nodeBlock,
                                     const uint16_t numberOfNodes,
                                     OutgoingMessageBuffer *outgoBuffer);

    void processNodes(NodeCluster *nodeCluster,
                      OutgoingMessageBuffer *outgoBuffer);
    void processAxons(EdgeCluster *edgeCluster,
                      OutgoingMessageBuffer *outgoBuffer);

    std::vector<uint8_t> m_sideOrder;
    NextChooser* m_nextChooser = nullptr;
};

}

#endif // CPUPROCESSINGUNIT_H
