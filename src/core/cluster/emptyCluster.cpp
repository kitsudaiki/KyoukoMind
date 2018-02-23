#include <core/cluster/emptyCluster.h>

namespace KyoChan_Network
{

EmptyCluster::EmptyCluster(quint32 clusterId,
                           Cluster *parentCluster)
    : Cluster(clusterId,
              EMPTYCLUSTER,
              parentCluster)
{

}

}
