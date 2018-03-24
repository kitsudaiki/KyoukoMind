#ifndef EDGECLUSTER_H
#define EDGECLUSTER_H

#include "cluster.h"

namespace KyoukoMind
{

class EdgeCluster : public Cluster
{

public:
    EdgeCluster(ClusterID clusterId,
                const std::string directoryPath);
};

}

#endif // EDGECLUSTER_H
