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

#include <core/clustering/cluster/cluster.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messages/replyMessage.h>

#include <core/processing/processingThreads/cpu/nextChooser.h>
#include <core/processing/processingThreads/cpu/edgeClusterProcessing.h>
#include <core/processing/processingThreads/cpu/nodeClusterProcessing.h>

#include <core/processing/processingThreads/cpu/processingMethods.h>

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

}

/**
 * @brief CpuProcessingUnit::processCluster
 * @param cluster
 */
void CpuProcessingUnit::processCluster(Cluster *cluster)
{
    //OUTPUT("processCluster")
    uint8_t clusterType = (uint8_t)cluster->getClusterType();

    switch((int)clusterType)
    {
        case EDGE_CLUSTER:
        {
            EdgeCluster *edgeCluster = static_cast<EdgeCluster*>(cluster);
            m_edgeProcessing->processIncomingMessages(edgeCluster);
            m_edgeProcessing->processAxons(edgeCluster);
            break;
        }
        case NODE_CLUSTER:
        {
            NodeCluster *nodeCluster = static_cast<NodeCluster*>(cluster);
            m_nodeProcessing->processDirectMessages(nodeCluster);
            m_nodeProcessing->processMessagesEdges(nodeCluster);
            m_nodeProcessing->processNodes(nodeCluster);
            m_nodeProcessing->processAxons(nodeCluster);
            break;
        }
        default:
            break;
    }
    cluster->finishCycle();
}

}
