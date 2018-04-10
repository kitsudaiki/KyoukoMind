/**
 *  @file    emptyCluster.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/cluster/emptyCluster.h>
#include <core/messaging/messageQueues/incomingMessageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

namespace KyoukoMind
{

/**
 * @brief EmptyCluster::EmptyCluster
 * @param clusterId
 * @param directoryPath
 * @param controller
 */
EmptyCluster::EmptyCluster(const ClusterID clusterId,
                           const std::string directoryPath,
                           MessageController *controller)
    : Cluster(clusterId,
              EMPTYCLUSTER,
              directoryPath)
{
    m_incomingMessageQueue = new IncomingMessageBuffer(clusterId, controller);
    m_outgoingMessageQueue = new OutgoingMessageBuffer(clusterId, controller);
}

/**
 * @brief EmptyCluster::EmptyCluster
 * @param clusterId
 * @param clusterType
 * @param directoryPath
 * @param controller
 */
EmptyCluster::EmptyCluster(const ClusterID clusterId,
                           const uint8_t clusterType,
                           const std::string directoryPath,
                           MessageController *controller)
    : Cluster(clusterId,
              clusterType,
              directoryPath)
{
    m_incomingMessageQueue = new IncomingMessageBuffer(clusterId, controller);
    m_outgoingMessageQueue = new OutgoingMessageBuffer(clusterId, controller);
}

}
