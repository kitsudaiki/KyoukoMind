/**
 *  @file    emptyCluster.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/clustering/cluster/emptyCluster.h>
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
                           const std::string directoryPath)
    : Cluster(clusterId,
              EMPTY_CLUSTER,
              directoryPath)
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
                           const std::string directoryPath)
    : Cluster(clusterId,
              clusterType,
              directoryPath)
{
}

/**
 * @brief EmptyCluster::finishCycle
 */
void EmptyCluster::finishCycle()
{
    m_outgoingMessageQueue->finishCycle(2);
    m_outgoingMessageQueue->finishCycle(3);
    m_outgoingMessageQueue->finishCycle(4);
    m_outgoingMessageQueue->finishCycle(11);
    m_outgoingMessageQueue->finishCycle(12);
    m_outgoingMessageQueue->finishCycle(13);
}

}
