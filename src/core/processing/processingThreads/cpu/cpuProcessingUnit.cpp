/**
 *  @file    cpuProcessingUnit.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/processing/processingThreads/cpu/cpuProcessingUnit.h>
#include <core/processing/processingThreads/cpu/nextChooser.h>

#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messages/replyMessage.h>

#include <core/processing/processingThreads/cpu/nextChooser.h>
#include <core/processing/processingThreads/cpu/clusterProcessing.h>

namespace KyoukoMind
{

/**
 * @brief CpuProcessingUnit::CpuProcessingUnit
 * @param clusterHandler
 */
CpuProcessingUnit::CpuProcessingUnit(ClusterQueue *clusterQueue):
    ProcessingUnit(clusterQueue)
{
    m_nextChooser = new NextChooser();
    m_clusterProcessing = new ClusterProcessing(m_nextChooser);
}

/**
 * @brief CpuProcessingUnit::~CpuProcessingUnit
 */
CpuProcessingUnit::~CpuProcessingUnit()
{
    delete m_clusterProcessing;
    delete m_nextChooser;
}

/**
 * @brief CpuProcessingUnit::processCluster
 * @param cluster
 */
void CpuProcessingUnit::processCluster(EdgeCluster *cluster)
{
    //OUTPUT("processCluster")
    uint8_t clusterType = (uint8_t)cluster->getClusterType();

    m_clusterProcessing->processMessagesEdges(cluster);
    if(clusterType == NODE_CLUSTER)
    {
        NodeCluster *nodeCluster = static_cast<NodeCluster*>(cluster);
        m_clusterProcessing->processNodes(nodeCluster);
    }
    cluster->finishCycle();
}

}
