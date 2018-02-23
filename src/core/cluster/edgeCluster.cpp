#include <core/cluster/edgeCluster.h>

namespace KyoChan_Network
{

EdgeCluster::EdgeCluster(quint32 clusterId,
                         Cluster *parentCluster)
    : Cluster(clusterId,
              EDGECLUSTER,
              parentCluster)
{

}

}
