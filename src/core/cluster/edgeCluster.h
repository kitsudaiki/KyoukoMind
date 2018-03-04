#ifndef EDGECLUSTER_H
#define EDGECLUSTER_H

#include "cluster.h"

namespace KyoukoMind
{

class EdgeCluster : public Cluster
{
    Q_OBJECT

public:
    EdgeCluster(ClusterID clusterId);
};

}

#endif // EDGECLUSTER_H
