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

#include <core/clustering/cluster/cluster.h>

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
    std::cout<<"poi0"<<std::endl;
    while(!m_abort)
    {
        std::cout<<"poi1"<<std::endl;
        if(m_block) {
            blockThread();
        }
        std::cout<<"poi2"<<std::endl;
        Cluster* cluster = m_clusterQueue->getCluster();
        if(cluster == nullptr)
        {
            // block thread until next cycle if queue is empty
            std::cout<<"poi3"<<std::endl;
            blockThread();
            std::cout<<"poi4"<<std::endl;
            for(uint32_t i = 0; i < m_finishClusterBuffer.size(); i++) {
                m_clusterQueue->addCluster(m_finishClusterBuffer[i]);
            }
            m_clusterQueue->clearQueue();
        } else {
            // process if ready or readd to queue if not ready
            std::cout<<"poi5"<<std::endl;
            if(cluster->isReady()) {
                processCluster(cluster);
                m_finishClusterBuffer.push_back(cluster);
            } else {
                m_clusterQueue->addCluster(cluster);
            }
        }
    }
}

}
