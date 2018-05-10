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
#include <core/processing/processingThreads/cpu/edgeClusterProcessing.h>
#include <core/processing/processingThreads/cpu/nodeClusterProcessing.h>

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
    m_edgeProcessing = new EdgeClusterProcessing(m_nextChooser);
    m_nodeProcessing = new NodeClusterProcessing(m_nextChooser);
}

/**
 * @brief CpuProcessingUnit::~CpuProcessingUnit
 */
CpuProcessingUnit::~CpuProcessingUnit()
{
    delete m_nodeProcessing;
    delete m_edgeProcessing;
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

    switch((int)clusterType)
    {
        case EDGE_CLUSTER:
        {
            m_edgeProcessing->processMessagesEdges(cluster);
            break;
        }
        case NODE_CLUSTER:
        {
            m_nodeProcessing->processMessagesEdges(cluster);
            NodeCluster *nodeCluster = static_cast<NodeCluster*>(cluster);
            m_nodeProcessing->processNodes(nodeCluster);
            break;
        }
        default:
            break;
    }
    cluster->finishCycle();
}

}
