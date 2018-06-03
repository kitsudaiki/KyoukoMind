/**
 *  @file    clusterProcessing.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef EDGECLUSTERPROCESSING_H
#define EDGECLUSTERPROCESSING_H

#include <common.h>
#include <core/structs/kyochanEdges.h>
#include <core/structs/kyochanNodes.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{
class OutgoingMessageBuffer;
class EdgeCluster;
class NodeCluster;

class EdgeClusterProcessing
{
public:
    EdgeClusterProcessing(PossibleKyoChanNodes* activeNodes);

    bool processMessagesEdgesCluster(EdgeCluster *cluster);

private:
    PossibleKyoChanNodes* m_activeNodes = nullptr;

    float randFloat(const float b);
    void learningForwardEdgeSection(EdgeCluster *cluster,
                                    KyoChanForwardEdgeSection *currentSection,
                                    const uint32_t forwardEdgeSectionId,
                                    const uint8_t inititalSide,
                                    const float partitialWeight,
                                    OutgoingMessageBuffer *outgoBuffer);
    void learningEdgeSection(KyoChanEdgeSection *currentSection,
                             const float partitialWeight);
    void updateEdgeForwardSection(EdgeCluster *cluster,
                                  const uint32_t forwardEdgeSectionId,
                                  const float status,
                                  const uint8_t inititalSide,
                                  OutgoingMessageBuffer *outgoBuffer);
    void processEdgeForwardSection(EdgeCluster *cluster,
                                   uint32_t forwardEdgeSectionId,
                                   const float weight,
                                   const uint8_t inititalSide,
                                   OutgoingMessageBuffer *outgoBuffer);
    void processEdgeSection(NodeCluster *cluster,
                            uint32_t edgeSectionId,
                            const float weight,
                            OutgoingMessageBuffer *outgoBuffer);
    void processAxon(EdgeCluster *cluster,
                     const uint32_t targetId,
                     const uint64_t path,
                     const float weight,
                     const uint8_t initialSide,
                     OutgoingMessageBuffer *outgoBuffer);
    void processLerningEdge(EdgeCluster *cluster,
                            const uint32_t sourceEdgeSectionId,
                            const float weight,
                            const uint8_t initSide,
                            OutgoingMessageBuffer *outgoBuffer);
};

}

#endif // EDGECLUSTERPROCESSING_H
