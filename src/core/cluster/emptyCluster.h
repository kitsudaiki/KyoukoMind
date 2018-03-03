#ifndef EMPTYCLUSTER_H
#define EMPTYCLUSTER_H

#include "cluster.h"

namespace KyoChan_Network
{

class EmptyCluster : public Cluster
{
    Q_OBJECT

public:
    EmptyCluster(ClusterID clusterId);

    void processCluster();

};

}

#endif // EMPTYCLUSTER_H
