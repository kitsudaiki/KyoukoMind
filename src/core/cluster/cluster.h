#ifndef CLUSTER_H
#define CLUSTER_H

#include <QObject>
#include <QVector>

#include <common/structs.h>

namespace Persistence
{
class IOBuffer;
}

namespace KyoChanMQ
{
class KyoChanMqClient;
}

namespace KyoChan_Network
{

class Cluster : public QObject
{
    Q_OBJECT

public:
    Cluster(ClusterID clusterId,
            ClusterType clusterType);
    ~Cluster();

    ClusterID getClusterId() const;
    ClusterType getClusterType() const;

    bool addNeighbor(const quint8 side, const quint32 targetId);

    // get network-pointer from buffer
    CluserMetaData* getMetaData();

    virtual void processCluster() = 0;


private:
    // cluster-metadata
    ClusterID m_clusterId;
    ClusterType m_clusterType = EMPTYCLUSTER;
    
protected:
    Persistence::IOBuffer* m_buffer = nullptr;
};

}

#endif // CLUSTER_H
