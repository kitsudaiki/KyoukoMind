#ifndef EDGECLUSTER_H
#define EDGECLUSTER_H

#include "cluster.h"

namespace KyoukoMind
{

class EdgeCluster : public Cluster
{
    Q_OBJECT

public:
    EdgeCluster(ClusterID clusterId,
                const QString directoryPath);
};

}

#endif // EDGECLUSTER_H
