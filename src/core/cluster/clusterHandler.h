#ifndef CLUSTERQUEUE_H
#define CLUSTERQUEUE_H

#include <QObject>
#include <QVector>
#include <QMap>

namespace KyoChan_Network
{

class Cluster;

class ClusterManager
{
public:
    ClusterManager();

    bool addCluster(const quint32 clusterId, Cluster* cluster);
    Cluster* getCluster(const quint32 clusterId);

private:
    QMap<quint32, Cluster*> m_allClusters;
};

}

#endif // CLUSTERQUEUE_H
