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
 * @brief ClusterQueue::~ClusterQueue
 */
ClusterQueue::~ClusterQueue()
{
    clearQueue();
}

/**
 * @brief ClusterQueue::clearQueue
 * @return
 */
bool ClusterQueue::clearQueue()
{
    if(m_queue.size() == 0) {
        return false;
    }
    std::queue<Cluster*> empty;
    std::swap(m_queue, empty);
    return true;
}

/**
 * @brief ClusterQueue::getCluster
 * @return
 */
Cluster *ClusterQueue::getCluster()
{
    if(m_queue.size() == 0) {
        return nullptr;
    }
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
