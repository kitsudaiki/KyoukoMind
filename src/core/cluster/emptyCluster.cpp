/**
 *  @file    emptyCluster.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/cluster/emptyCluster.h>
#include <core/messaging/messageController.h>
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
              EMPTY_CLUSTER,
              directoryPath,
              controller)
{

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
              directoryPath,
              controller)
{
}

/**
 * @brief EmptyCluster::finishCycle
 */
void EmptyCluster::finishCycle()
{
    m_outgoingMessageQueue->sendFinishCycle(2);
    m_outgoingMessageQueue->sendFinishCycle(3);
    m_outgoingMessageQueue->sendFinishCycle(4);
    m_outgoingMessageQueue->sendFinishCycle(11);
    m_outgoingMessageQueue->sendFinishCycle(12);
    m_outgoingMessageQueue->sendFinishCycle(13);
}

}
