/**
 *  @file    cpuProcessingUnit.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/processing/cpu/cpuProcessingUnit.h>

#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/processing/cpu/edgeClusterProcessing.h>
#include <core/processing/cpu/nodeClusterProcessing.h>

namespace KyoukoMind
{

/**
 * @brief CpuProcessingUnit::CpuProcessingUnit
 * @param clusterHandler
 */
CpuProcessingUnit::CpuProcessingUnit(ClusterQueue *clusterQueue):
    ProcessingUnit(clusterQueue)
{
    m_edgeClusterProcessing = new EdgeClusterProcessing();
    m_nodeClusterProcessing = new NodeClusterProcessing();
}

/**
 * @brief CpuProcessingUnit::~CpuProcessingUnit
 */
CpuProcessingUnit::~CpuProcessingUnit()
{
    delete m_edgeClusterProcessing;
}

/**
 * @brief CpuProcessingUnit::processCluster process of a cluster in one cycle
 * @param cluster custer which should be processed
 */
void CpuProcessingUnit::processCluster(Cluster *cluster)
{
    uint8_t clusterType = (uint8_t)cluster->getClusterType();

    uint16_t numberOfActiveNodes = 0;

    // process nodes if cluster is a node-cluster
    if(clusterType == NODE_CLUSTER)
    {
        NodeCluster *nodeCluster = static_cast<NodeCluster*>(cluster);
        numberOfActiveNodes = m_nodeClusterProcessing->processNodes(nodeCluster);
        m_nodeClusterProcessing->processMessagesNodeCluster(nodeCluster);
    }
    else
    {
        EdgeCluster *edgeCluster = static_cast<EdgeCluster*>(cluster);
        m_edgeClusterProcessing->processMessagesEdgesCluster(edgeCluster);
    }

    // finish the processing-cycle of the current cluster
    cluster->finishCycle(numberOfActiveNodes);
}

}
