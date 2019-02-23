/**
 *  @file    processingUnit.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/processing/processingUnit.h>
#include <core/clustering/clusterHandler.h>
#include <core/clustering/clusterQueue.h>

#include <core/clustering/cluster/cluster.h>
#include <core/clustering/globalValuesHandler.h>

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
void
ProcessingUnit::run()
{
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    while(!m_abort)
    {
        if(m_block) {
            blockThread();
        }
        Cluster* cluster = m_clusterQueue->getCluster();
        if(cluster == nullptr)
        {
            end = std::chrono::system_clock::now();
            std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << '\n';

            // block thread until next cycle if queue is empty
            blockThread();
            start = std::chrono::system_clock::now();

            for(uint32_t i = 0; i < m_finishClusterBuffer.size(); i++) {
                m_clusterQueue->addCluster(m_finishClusterBuffer[i]);
            }
            m_finishClusterBuffer.clear();
            GlobalValues globalValues = m_clusterQueue->getGlobalValuesHandler()->getGlobalValues();
            m_clusterQueue->getGlobalValuesHandler()->setGlobalValues(globalValues);
        }
        else
        {
            // process if ready or readd to queue if not ready
            if(cluster->isBufferReady()) {
                processCluster(cluster);
                m_finishClusterBuffer.push_back(cluster);
            } else {
                m_clusterQueue->addCluster(cluster);
            }
        }
    }
}

}
