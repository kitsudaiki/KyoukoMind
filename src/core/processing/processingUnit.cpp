/**
 *  @file    processingUnit.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/processing/processingUnit.h>
#include <core/cluster/clusterHandler.h>
#include <core/cluster/clusterQueue.h>

#include <core/cluster/cluster.h>

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

        Cluster* cluster = m_clusterQueue->getCluster();
        // TODO: check if cluster is ready for processing
        processCluster(cluster);
        m_clusterQueue->addCluster(cluster);
    }
}


}
