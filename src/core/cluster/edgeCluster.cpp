#include <core/cluster/edgeCluster.h>

namespace KyoukoMind
{

EdgeCluster::EdgeCluster(ClusterID clusterId,
                         const std::string directoryPath)
    : Cluster(clusterId,
              EDGECLUSTER,
              directoryPath)
{

}

}
