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

#include <core/cluster/cluster.h>
#include <core/cluster/emptyCluster.h>
#include <core/cluster/edgeCluster.h>
#include <core/cluster/nodeCluster.h>

#include <core/messaging/messages/message.h>
#include <core/messaging/messages/dataMessage.h>
#include <core/messaging/messages/replyMessage.h>

#include <core/processing/processingThreads/cpu/nextChooser.h>
#include <core/processing/processingThreads/cpu/axonprocessing.h>
#include <core/processing/processingThreads/cpu/nodeprocessing.h>
#include <core/processing/processingThreads/cpu/edgeprocessing.h>

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
    m_axonProcessing = new AxonProcessing(m_nextChooser);
    m_nodeProcessing = new NodeProcessing();
    m_edgeProcessing = new EdgeProcessing(m_nextChooser);
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
    uint8_t clusterType = (uint8_t)cluster->getClusterType();

    switch((int)clusterType)
    {
        case EMPTY_CLUSTER:
            ((EmptyCluster*)cluster)->finishCycle();
            break;
        case EDGE_CLUSTER:
        {
            EdgeCluster *edgeCluster = (EdgeCluster*)cluster;
            m_axonProcessing->processAxons(edgeCluster);
            m_edgeProcessing->processIncomingMessages(edgeCluster);
            edgeCluster->getPendingEdges()->checkPendingEdges();
            edgeCluster->finishCycle();
            break;
        }
        case NODE_CLUSTER:
        {
            NodeCluster *nodeCluster = (NodeCluster*)cluster;
            m_edgeProcessing->processInputMessages(nodeCluster);
            m_nodeProcessing->processNodes(nodeCluster);
            m_axonProcessing->processAxons((EdgeCluster*)nodeCluster);
            m_edgeProcessing->processIncomingMessages((EdgeCluster*)nodeCluster);
            nodeCluster->getPendingEdges()->checkPendingEdges();
            nodeCluster->finishCycle();
            break;
        }
        default:
            break;
    }
}

}
