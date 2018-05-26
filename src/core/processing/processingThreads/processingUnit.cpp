/**
 *  @file    processingUnit.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/processing/processingThreads/processingUnit.h>
#include <core/clustering/clusterHandler.h>
#include <core/clustering/clusterQueue.h>

#include <core/clustering/cluster/edgeCluster.h>

namespace KyoukoMind
{

/**
 * @brief ProcessingUnit::ProcessingUnit
 * @param clusterHandler
 */
ProcessingUnit::ProcessingUnit(ClusterQueue* clusterQueue)
{
    m_clusterQueue = clusterQueue;
    m_block = true;
}

/**
 * @brief ProcessingUnit::run
 */
void ProcessingUnit::run()
{
    while(!m_abort)
    {
        if(m_block) {
            blockThread();
        }
        EdgeCluster* cluster = m_clusterQueue->getCluster();
        if(cluster == nullptr)
        {
            // block thread until next cycle if queue is empty
            blockThread();
            for(uint32_t i = 0; i < m_finishClusterBuffer.size(); i++) {
                m_clusterQueue->addCluster(m_finishClusterBuffer[i]);
            }
            m_finishClusterBuffer.clear();
        } else {
            // process if ready or readd to queue if not ready

            // TODO: check if cluster is ready
            if(true) {
                processCluster(cluster);
                m_finishClusterBuffer.push_back(cluster);
            } else {
                m_clusterQueue->addCluster(cluster);
            }
        }
    }
}

}
