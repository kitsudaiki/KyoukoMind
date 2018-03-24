#include <core/cluster/emptyCluster.h>

namespace KyoukoMind
{

EmptyCluster::EmptyCluster(ClusterID clusterId,
                           const std::string directoryPath)
    : Cluster(clusterId,
              EMPTYCLUSTER,
              directoryPath)
{

}

}
