/**
 *  @file    clusterQueue.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "clusterQueue.h"

namespace KyoukoMind
{

/**
 * @brief ClusterQueue::ClusterQueue
 */
ClusterQueue::ClusterQueue()
{

}

/**
 * @brief ClusterQueue::getCluster
 * @return
 */
Cluster *ClusterQueue::getCluster()
{
    Cluster* tempCluster = m_queue.front();
    m_queue.pop();
    return tempCluster;
}

/**
 * @brief ClusterQueue::addCluster
 * @param cluster
 * @return
 */
bool ClusterQueue::addCluster(Cluster *cluster)
{
    m_queue.push(cluster);
    return true;
}

}
