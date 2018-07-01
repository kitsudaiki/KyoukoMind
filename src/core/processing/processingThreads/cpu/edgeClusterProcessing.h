/**
 *  @file    edgeClusterProcessing.h
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
#include <core/structs/clusterMeta.h>
#include <core/structs/messageContainer.h>

namespace Networking
{
class OutgoingMessageBuffer;
}

namespace KyoukoMind
{
class EdgeCluster;
class NodeCluster;

class EdgeClusterProcessing
{
public:
    EdgeClusterProcessing();

    bool processMessagesEdgesCluster(EdgeCluster *cluster);

private:
    float m_weightMap[17];

    void learningForwardEdgeSection(EdgeCluster *cluster,
                                    KyoChanForwardEdgeSection *currentSection,
                                    const uint32_t forwardEdgeSectionId,
                                    const float partitialWeight,
                                    Networking::OutgoingMessageBuffer *outgoBuffer);
    void learningEdgeSection(KyoChanEdgeSection *currentSection,
                             const float partitialWeight);
    void updateEdgeForwardSection(EdgeCluster *cluster,
                                  const uint32_t forwardEdgeSectionId,
                                  const float status,
                                  const uint8_t inititalSide,
                                  Networking::OutgoingMessageBuffer *outgoBuffer);
    void processEdgeForwardSection(EdgeCluster *cluster,
                                   uint32_t forwardEdgeSectionId,
                                   const float weight,
                                   Networking::OutgoingMessageBuffer *outgoBuffer);
    void processEdgeSection(NodeCluster *cluster,
                            uint32_t edgeSectionId,
                            const float weight,
                            Networking::OutgoingMessageBuffer *outgoBuffer);
    void processAxon(EdgeCluster *cluster,
                     const uint32_t targetId,
                     const uint64_t path,
                     const float weight,
                     Networking::OutgoingMessageBuffer *outgoBuffer);
    void processLerningEdge(EdgeCluster *cluster,
                            const uint32_t sourceEdgeSectionId,
                            const float weight,
                            const uint8_t initSide,
                            Networking::OutgoingMessageBuffer *outgoBuffer);
    void processPendingEdge(EdgeCluster* cluster,
                            const uint32_t sourceId,
                            const uint8_t sourceSide,
                            const float weight,
                            Networking::OutgoingMessageBuffer *outgoBuffer);

    void refillWeightMap(const uint8_t initialSide, Neighbor* neighbors);
    float randFloat(const float b);

};

}

#endif // EDGECLUSTERPROCESSING_H
