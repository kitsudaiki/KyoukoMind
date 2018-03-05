#include <core/cluster/edgeCluster.h>

namespace KyoukoMind
{

EdgeCluster::EdgeCluster(ClusterID clusterId,
                         const QString directoryPath)
    : Cluster(clusterId,
              EDGECLUSTER,
              directoryPath,
              0)
{

}

}
