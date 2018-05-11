/**
 *  @file    clusterHandler.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef CLUSTERHANDLER_H
#define CLUSTERHANDLER_H

#include <common.h>

namespace KyoukoMind
{
class EdgeCluster;
class ClusterQueue;

class ClusterHandler
{
public:
    ClusterHandler();
    ~ClusterHandler();

    bool addCluster(const ClusterID clusterId, EdgeCluster* cluster);
    EdgeCluster* getCluster(const ClusterID clusterId);
    bool deleteCluster(const ClusterID clusterId);
    void clearAllCluster();

    ClusterQueue* getClusterQueue() const;

private:
    std::map<ClusterID, EdgeCluster*> m_allClusters;

    ClusterQueue* m_clusterQueue = nullptr;
};

}

#endif // CLUSTERHANDLER_H
