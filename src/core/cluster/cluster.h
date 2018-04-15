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
            const std::string directoryPath,
            MessageController *controller);
    ~Cluster();

    ClusterID getClusterId() const;
    uint8_t getClusterType() const;

    bool isReady() const;
    IncomingMessageBuffer* getIncomingMessageBuffer() const;
    OutgoingMessageBuffer* getOutgoingMessageBuffer() const;

    bool addNeighbor(const uint8_t side, const Neighbor target);
    Neighbor *getNeighbors();
    ClusterID getNeighborId(const uint8_t side);

    void getMetaData();
    void updateMetaData(ClusterMetaData metaData);

private:
    // cluster-metadata
    uint64_t m_messageIdCounter = 0;

    void initMessageBuffer(MessageController *controller);
    
protected:
    PerformanceIO::DataBuffer* m_buffer = nullptr;
    IncomingMessageBuffer* m_incomingMessageQueue = nullptr;
    OutgoingMessageBuffer* m_outgoingMessageQueue = nullptr;
    ClusterMetaData m_metaData;

    void initFile(const ClusterID clusterId,
                  const std::string directoryPath);
};

}

#endif // CLUSTER_H
