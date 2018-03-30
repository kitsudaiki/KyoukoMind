#include <core/cluster/emptyCluster.h>
#include <core/messaging/messageQueues/incomingmessagequeue.h>
#include <core/messaging/messageQueues/outgoingmessagequeue.h>

namespace KyoukoMind
{

EmptyCluster::EmptyCluster(ClusterID clusterId,
                           const std::string directoryPath,
                           MessageController *controller)
    : Cluster(clusterId,
              directoryPath)
{
    m_clusterType = EMPTYCLUSTER;

    m_incomingMessageQueue = new IncomingMessageQueue(clusterId, controller);
    m_outgoingMessageQueue = new OutgoingMessageQueue(clusterId, controller);
}

}
