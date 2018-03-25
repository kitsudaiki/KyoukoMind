#include <core/cluster/edgeCluster.h>

namespace KyoukoMind
{

EdgeCluster::EdgeCluster(ClusterID clusterId,
                         const std::string directoryPath)
    : EmptyCluster(clusterId,
                   directoryPath)
{
    m_clusterType = EDGECLUSTER;
}

}
