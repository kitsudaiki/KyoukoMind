#include <core/cluster/emptyCluster.h>

namespace KyoukoMind
{

EmptyCluster::EmptyCluster(ClusterID clusterId,
                           const QString directoryPath)
    : Cluster(clusterId,
              EMPTYCLUSTER,
              directoryPath)
{

}

}
