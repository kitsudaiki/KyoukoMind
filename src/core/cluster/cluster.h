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

enum ClusterType
{
    EMPTYCLUSTER = 0,
    EDGECLUSTER = 1,
    NODECLUSTER = 2
};

class Cluster : public QObject
{
    Q_OBJECT

public:
    Cluster(quint32 clusterId,
            ClusterType clusterType,
            Cluster* parentCluster = nullptr);
    ~Cluster();

    Cluster* getChildCluster(quint8 clusterId);
    Cluster* getParentCluster();

    quint32 getClusterId() const;
    ClusterType getClusterType() const;
    quint16 getClusterLevel() const;

    bool addNeighbor(const quint8 side, const quint32 targetId);

    // get network-pointer from buffer
    KyoChanMetaData* getMetaData();


private:
    // cluster-metadata
    quint32 m_clusterId = 0;
    ClusterType m_clusterType = EMPTYCLUSTER;
    quint16 m_clusterLevel = 0;

    // cluster-hirarchie-pointer
    Cluster* m_parentCluster = nullptr;
    Cluster* m_childClusterIds[7];
    
protected:
    Persistence::IOBuffer* m_buffer = nullptr;
};

}

#endif // CLUSTER_H
