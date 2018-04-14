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
              EMPTYCLUSTER,
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
    m_outgoingMessageQueue->sendFinishCycle(m_metaData.clusterId, 2);
    m_outgoingMessageQueue->sendFinishCycle(m_metaData.clusterId, 3);
    m_outgoingMessageQueue->sendFinishCycle(m_metaData.clusterId, 4);
    m_outgoingMessageQueue->sendFinishCycle(m_metaData.clusterId, 11);
    m_outgoingMessageQueue->sendFinishCycle(m_metaData.clusterId, 12);
    m_outgoingMessageQueue->sendFinishCycle(m_metaData.clusterId, 13);
}

}
