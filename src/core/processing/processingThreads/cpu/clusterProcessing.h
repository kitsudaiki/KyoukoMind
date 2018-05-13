/**
 *  @file    clusterProcessing.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef CLUSTERPROCESSING_H
#define CLUSTERPROCESSING_H

#include <common.h>
#include <core/structs/kyochanEdges.h>
#include <core/structs/kyochanNodes.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{
class OutgoingMessageBuffer;
class EdgeCluster;
class NodeCluster;
class NextChooser;

class ClusterProcessing
{
public:
    ClusterProcessing(NextChooser* nextChooser,
                      PossibleKyoChanNodes* activeNodes);

    void initLearing(EdgeCluster *cluster,
                     const uint32_t forwardEdgeSectionId,
                     const uint8_t inititalSide,
                     const float weightDiff,
                     OutgoingMessageBuffer* outgoBuffer);

    void processEdgeForwardSection(EdgeCluster *cluster,
                                   const uint32_t forwardEdgeSectionId,
                                   const float weight,
                                   const uint8_t inititalSide,
                                   OutgoingMessageBuffer *outgoBuffer);
    void processEdgeSection(NodeCluster *cluster,
                            uint32_t edgeSectionId,
                            const float weight);

private:
    NextChooser* m_nextChooser = nullptr;
    PossibleKyoChanNodes* m_activeNodes = nullptr;

    float randFloat(const float b);
    void learningExternalForwardEdge(KyoChanForwardEdgeSection *currentSection,
                                     const uint32_t forwardEdgeSectionId,
                                     const uint8_t side,
                                     const float partitialWeight,
                                     OutgoingMessageBuffer *outgoBuffer);
    void learningInternalForwardEdge(KyoChanForwardEdgeSection *currentSection,
                                     EdgeCluster *cluster,
                                     const float partitialWeight);
};

}

#endif // CLUSTERPROCESSING_H
