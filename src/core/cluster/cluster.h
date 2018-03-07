#ifndef CLUSTER_H
#define CLUSTER_H

#include <QObject>
#include <QVector>

#include <common/structs.h>

namespace Persistence
{
class IOBuffer;
}

namespace KyoukoMind
{

class Cluster : public QObject
{
    Q_OBJECT

public:
    Cluster(ClusterID clusterId,
            ClusterType clusterType, const QString directoryPath);
    ~Cluster();

    ClusterID getClusterId() const;
    ClusterType getClusterType() const;

    bool addNeighbor(const quint8 side, const Neighbor target);

    void getMetaData();
    void updateMetaData(ClusterMetaData metaData);
    ClusterID convertId(const quint32 clusterId);

private:
    // cluster-metadata
    ClusterID m_clusterId;
    ClusterType m_clusterType = EMPTYCLUSTER;
    
protected:
    Persistence::IOBuffer* m_buffer = nullptr;
    ClusterMetaData m_metaData;

    void initFile(const ClusterID clusterId,
                  const QString directoryPath);
};

}

#endif // CLUSTER_H
