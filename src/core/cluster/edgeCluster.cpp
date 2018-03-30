#include <core/cluster/edgeCluster.h>

namespace KyoukoMind
{

EdgeCluster::EdgeCluster(ClusterID clusterId,
                         const std::string directoryPath,
                         MessageController *controller)
    : EmptyCluster(clusterId,
                   directoryPath,
                   controller)
{
    m_clusterType = EDGECLUSTER;
}

}
