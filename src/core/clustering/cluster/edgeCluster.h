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

    // axons
    uint32_t getNumberOfAxons() const;
    uint32_t getNumberOfAxonBlocks() const;
    KyoChanAxon* getAxonBlock();
    bool initAxonBlocks(const uint32_t numberOfAxons);

    // edge-forward-sections
    uint32_t getNumberOfEdgeBlocks() const;
    KyoChanForwardEdgeSection* getEdgeBlock();
    bool initEdgeBlocks(const uint32_t numberOfEdgeSections);
    uint32_t addEmptyEdgeSection();

    // finisher
    void finishCycle();

    // persistency
    void syncEdgeSections(uint32_t startSection = 0,
                          uint32_t endSection = 0);


protected:
    PerformanceIO::DataBuffer* m_clusterDataBuffer = nullptr;
    IncomingMessageBuffer* m_incomingMessageQueue = nullptr;
    OutgoingMessageBuffer* m_outgoingMessageQueue = nullptr;
    ClusterMetaData m_metaData;
};

}

#endif // CLUSTER_H
