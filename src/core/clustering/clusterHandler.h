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

namespace Networking
{
class IncomingMessageBuffer;
}

namespace KyoukoMind
{
class Cluster;
class ClusterQueue;

class ClusterHandler
{
public:
    ClusterHandler();
    ~ClusterHandler();

    bool addCluster(const ClusterID clusterId,
                    Cluster* cluster,
                    bool addToQueue = true);
    bool addCluster(const ClusterID targetClusterId,
                    const uint8_t targetSide,
                    Cluster* cluster,
                    bool addToQueue = true);
    Cluster* getCluster(const ClusterID clusterId);
    Cluster* getClusterByIndex(const uint32_t index);
    uint32_t getNumberOfCluster() const;
    bool deleteCluster(const ClusterID clusterId);
    void clearAllCluster();

    bool setNewConnection(const ClusterID targetClusterId,
                          const uint8_t targetSide,
                          const ClusterID sourceClusterId,
                          const bool bidirect = true);

    ClusterQueue* getClusterQueue() const;

private:
    std::map<ClusterID, Cluster*> m_allClusters;

    ClusterQueue* m_clusterQueue = nullptr;
};

}

#endif // CLUSTERHANDLER_H
