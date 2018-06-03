/**
 *  @file    clusterProcessing.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef NODECLUSTERPROCESSING_H
#define NODECLUSTERPROCESSING_H

#include <common.h>
#include <core/structs/kyochanEdges.h>
#include <core/structs/kyochanNodes.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{
class OutgoingMessageBuffer;
class EdgeCluster;
class NodeCluster;

class NodeClusterProcessing
{
public:
    NodeClusterProcessing(PossibleKyoChanNodes* activeNodes);

    bool processMessagesNodeCluster(NodeCluster *cluster);
    uint16_t processNodes(NodeCluster *nodeCluster);

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
                                   const uint32_t forwardEdgeSectionId,
                                   const float weight,
                                   const uint8_t inititalSide,
                                   OutgoingMessageBuffer *outgoBuffer);
    void processEdgeSection(NodeCluster *cluster,
                            uint32_t edgeSectionId,
                            const float weight,
                            OutgoingMessageBuffer *outgoBuffer);

};

}

#endif // NODECLUSTERPROCESSING_H
