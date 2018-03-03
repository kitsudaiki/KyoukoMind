#ifndef CLUSTERQUEUE_H
#define CLUSTERQUEUE_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <common/structs.h>

namespace KyoukoMind
{

class Cluster;

class ClusterManager
{
public:
    ClusterManager();

    bool addCluster(const ClusterID clusterId, Cluster* cluster);
    Cluster* getCluster(const ClusterID clusterId);

private:
    QMap<ClusterID, Cluster*> m_allClusters;
};

}

#endif // CLUSTERQUEUE_H
