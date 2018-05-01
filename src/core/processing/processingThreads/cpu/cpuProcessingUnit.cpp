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
    //OUTPUT("processCluster")
    uint8_t clusterType = (uint8_t)cluster->getClusterType();

    switch((int)clusterType)
    {
        case EDGE_CLUSTER:
        {
            EdgeCluster *edgeCluster = static_cast<EdgeCluster*>(cluster);
            m_edgeProcessing->processIncomingMessages(edgeCluster);
            m_edgeProcessing->processAxons(edgeCluster);
            edgeCluster->finishCycle();
            break;
        }
        case NODE_CLUSTER:
        {
            NodeCluster *nodeCluster = static_cast<NodeCluster*>(cluster);
            m_edgeProcessing->processInputMessages(nodeCluster);
            m_edgeProcessing->processIncomingMessages((EdgeCluster*)nodeCluster);
            m_edgeProcessing->processNodes(nodeCluster);
            m_edgeProcessing->processAxons((EdgeCluster*)nodeCluster);
            nodeCluster->finishCycle();
            break;
        }
        default:
            break;
    }
}

}
