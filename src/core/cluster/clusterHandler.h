/**
 *  @file    clusterHandler.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef CLUSTERQUEUE_H
#define CLUSTERQUEUE_H

#include <common.h>

namespace KyoukoMind
{
class Cluster;
class ClusterQueue;

class ClusterHandler
{
public:
    ClusterHandler();
    ~ClusterHandler();

    bool addCluster(const ClusterID clusterId, Cluster* cluster);
    Cluster* getCluster(const ClusterID clusterId);
    bool deleteCluster(const ClusterID clusterId);
    void clearAllCluster();

    ClusterQueue* getClusterQueue() const;

private:
    std::map<ClusterID, Cluster*> m_allClusters;

    ClusterQueue* m_clusterQueue = nullptr;
};

}

#endif // CLUSTERQUEUE_H
