/**
 *  @file    edgeCluster.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef CLUSTER_H
#define CLUSTER_H

#include <common.h>
#include <core/structs/clusterMeta.h>
#include <core/structs/kyochanEdges.h>
#include <core/structs/kyochanNodes.h>

namespace PerformanceIO
{
class DataBuffer;
}

namespace KyoukoMind
{
class MessageController;
class IncomingMessageBuffer;
class OutgoingMessageBuffer;

class EdgeCluster
{

public:
    EdgeCluster(const ClusterID &clusterId,
                const std::string directoryPath);
    ~EdgeCluster();

    // metadata
    void getMetaData();
    void updateMetaData();

    // common getter
    ClusterID getClusterId() const;
    uint8_t getClusterType() const;

    // message-queues
    void initMessageBuffer(MessageController *controller);
    IncomingMessageBuffer* getIncomingMessageBuffer();
    OutgoingMessageBuffer* getOutgoingMessageBuffer();

    // neighbors
    bool addNeighbor(const uint8_t side, const Neighbor target);
    Neighbor *getNeighbors();
    ClusterID getNeighborId(const uint8_t side);

    // edge-forward-sections
    uint32_t getNumberOfForwardEdgeSectionBlocks() const;
    KyoChanForwardEdgeSection* getForwardEdgeSectionBlock();
    bool initForwardEdgeSectionBlocks(const uint32_t numberOfForwardEdgeSections);
    uint32_t addEmptyForwardEdgeSection();

    // pending-forward-edge
    KyoChanForwardEdgeSection *getPendingForwardEdgeSectionBlock();
    uint32_t getPendingForwardEdgeSectionId() const;
    void decreaseNumberOfPendingForwardEdges();

    // finisher
    void finishCycle(const uint16_t numberOfActiveNodes);

protected:
    PerformanceIO::DataBuffer* m_clusterDataBuffer = nullptr;
    IncomingMessageBuffer* m_incomingMessageQueue = nullptr;
    OutgoingMessageBuffer* m_outgoingMessageQueue = nullptr;
    ClusterMetaData m_metaData;
};

}

#endif // CLUSTER_H
