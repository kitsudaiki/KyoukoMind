#ifndef CLUSTERQUEUE_H
#define CLUSTERQUEUE_H

#include <QVector>
#include <QMap>
#include <iterator>
#include <common.h>

namespace KyoukoMind
{

class Cluster;

class ClusterHandler
{
public:
    ClusterHandler();
    ~ClusterHandler();

    bool addCluster(const ClusterID clusterId, Cluster* cluster);
    Cluster* getCluster(const ClusterID clusterId);
    bool deleteCluster(const ClusterID clusterId);
    void clearAllCluster();

private:
    QMap<ClusterID, Cluster*> m_allClusters;
};

}

#endif // CLUSTERQUEUE_H
