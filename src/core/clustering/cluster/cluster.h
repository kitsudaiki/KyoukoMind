/**
 *  @file    cluster.h
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

class Cluster
{

public:
    Cluster(const ClusterID &clusterId,
            const uint8_t clusterType,
            const std::string directoryPath);
    ~Cluster();

    ClusterID getClusterId() const;
    uint8_t getClusterType() const;

    bool isReady() const;
    IncomingMessageBuffer* getIncomingMessageBuffer();
    OutgoingMessageBuffer* getOutgoingMessageBuffer();

    bool addNeighbor(const uint8_t side, const Neighbor target);
    Neighbor *getNeighbors();
    ClusterID getNeighborId(const uint8_t side);
    uint32_t getNextNewEdgeId();

    void getMetaData();
    void updateMetaData(ClusterMetaData metaData);

    void initMessageBuffer(MessageController *controller);

    uint32_t getNumberOfAxons() const;
    uint32_t getNumberOfAxonBlocks() const;

    KyoChanAxon* getAxonBlock();
    bool initAxonBlocks(const uint32_t numberOfAxons);

    void finishCycle();

    void syncEdgeSections(uint32_t startSection = 0,
                          uint32_t endSection = 0);

    virtual bool initEdgeBlocks(const uint32_t numberOfEdgeSections) = 0;

private:
    // cluster-metadata
    uint64_t m_messageIdCounter = 0;
    uint32_t m_newEdgeIdCounter = 0;

protected:
    PerformanceIO::DataBuffer* m_clusterDataBuffer = nullptr;
    IncomingMessageBuffer* m_incomingMessageQueue = nullptr;
    OutgoingMessageBuffer* m_outgoingMessageQueue = nullptr;
    ClusterMetaData m_metaData;

    void initFile(const ClusterID clusterId,
                  const std::string directoryPath);
};

}

#endif // CLUSTER_H
