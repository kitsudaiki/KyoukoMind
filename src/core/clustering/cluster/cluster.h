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

namespace Networking
{
class IncomingMessageBuffer;
class OutgoingMessageBuffer;
}

namespace KyoukoMind
{

class Cluster
{
public:
    Cluster(const ClusterID &clusterId,
            const std::string directoryPath);
    ~Cluster();

    // metadata
    ClusterMetaData getMetaData() const;
    void getMetaDataFromBuffer();
    void updateMetaData();

    // common getter
    ClusterID getClusterId() const;
    uint8_t getClusterType() const;

    // message-queues
    Networking::IncomingMessageBuffer* getIncomingMessageBuffer(const uint8_t side);
    Networking::OutgoingMessageBuffer* getOutgoingMessageBuffer(const uint8_t side);
    bool setIncomingMessageBuffer(const uint8_t side, Networking::IncomingMessageBuffer* buffer);

    // neighbors
    bool addNeighbor(const uint8_t side, const ClusterID targetClusterId);
    Neighbor *getNeighbors();
    ClusterID getNeighborId(const uint8_t side);

    // finisher
    void finishCycle(const uint16_t numberOfActiveNodes);

protected:
    PerformanceIO::DataBuffer* m_clusterDataBuffer = nullptr;
    ClusterMetaData m_metaData;
};

}

#endif // CLUSTER_H
