#ifndef EDGECLUSTER_H
#define EDGECLUSTER_H

#include "cluster.h"

namespace KyoChan_Network
{

class EdgeCluster : public Cluster
{
    Q_OBJECT

public:
    EdgeCluster(ClusterID clusterId);

    void processCluster();

};

}

#endif // EDGECLUSTER_H
