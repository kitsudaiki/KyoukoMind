#ifndef CLUSTER_H
#define CLUSTER_H

#include <common.h>

namespace Persistence
{
class IOBuffer;
}

namespace KyoukoMind
{

class Cluster
{

public:
    Cluster(const ClusterID &clusterId,
            const ClusterType clusterType,
            const std::string directoryPath);
    ~Cluster();

    ClusterID getClusterId() const;
    ClusterType getClusterType() const;

    bool addNeighbor(const uint8_t side, const Neighbor target);

    void getMetaData();
    void updateMetaData(ClusterMetaData metaData);

private:
    // cluster-metadata
    uint64_t m_messageIdCounter = 0;
    ClusterID m_clusterId;
    ClusterType m_clusterType = EMPTYCLUSTER;
    
protected:
    Persistence::IOBuffer* m_buffer = nullptr;
    ClusterMetaData m_metaData;

    void initFile(const ClusterID clusterId,
                  const std::string directoryPath);
};

}

#endif // CLUSTER_H
