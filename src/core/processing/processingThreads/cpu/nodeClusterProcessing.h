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

namespace Networking
{
class OutgoingMessageBuffer;
}

namespace KyoukoMind
{
class EdgeCluster;
class NodeCluster;

class NodeClusterProcessing
{
public:
    NodeClusterProcessing();

    bool processMessagesNodeCluster(NodeCluster *cluster);
    uint16_t processNodes(NodeCluster *nodeCluster);

private:
    float randFloat(const float b);
    void learningEdgeSection(NodeCluster *cluster,
                             KyoChanEdgeSection *currentSection,
                             const float partitialWeight);
    void processLerningEdge(NodeCluster *cluster,
                            const uint32_t sourceEdgeSectionId,
                            const float weight,
                            const uint8_t initSide,
                            Networking::OutgoingMessageBuffer *outgoBuffer);
    void processPendingEdge(NodeCluster *cluster,
                            const uint32_t sourceId,
                            const uint8_t sourceSide,
                            const float weight,
                            Networking::OutgoingMessageBuffer *outgoBuffer);
    void processEdgeSection(NodeCluster *cluster,
                            uint32_t edgeSectionId,
                            const float weight,
                            Networking::OutgoingMessageBuffer *outgoBuffer);
};

}

#endif // NODECLUSTERPROCESSING_H