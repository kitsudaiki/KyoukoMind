#include <core/cluster/emptyCluster.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

namespace KyoukoMind
{

EmptyCluster::EmptyCluster(ClusterID clusterId,
                           const std::string directoryPath,
                           MessageController *controller)
    : Cluster(clusterId,
              directoryPath)
{
    m_clusterType = EMPTYCLUSTER;

    m_incomingMessageQueue = new IncomingMessageBuffer(clusterId, controller);
    m_outgoingMessageQueue = new OutgoingMessageBuffer(clusterId, controller);
}

}
