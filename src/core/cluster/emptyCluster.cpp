#include <core/cluster/emptyCluster.h>

namespace KyoChan_Network
{

EmptyCluster::EmptyCluster(ClusterID clusterId)
    : Cluster(clusterId,
              EMPTYCLUSTER)
{

}

void EmptyCluster::processCluster()
{

}

}
