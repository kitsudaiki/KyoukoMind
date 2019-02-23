/**
 *  @file    cpuProcessingUnit.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/processing/cpuProcessingUnit.h>

#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

namespace KyoukoMind
{

/**
 * @brief CpuProcessingUnit::CpuProcessingUnit
 * @param clusterHandler
 */
CpuProcessingUnit::CpuProcessingUnit(ClusterQueue *clusterQueue):
    ProcessingUnit(clusterQueue)
{

}

/**
 * @brief CpuProcessingUnit::~CpuProcessingUnit
 */
CpuProcessingUnit::~CpuProcessingUnit()
{
}

/**
 * @brief CpuProcessingUnit::processCluster process of a cluster in one cycle
 * @param cluster custer which should be processed
 */
void
CpuProcessingUnit::processCluster(Cluster *cluster)
{
    // process nodes if cluster is a node-cluster
    if(cluster->getClusterType() == NODE_CLUSTER)
    {
        NodeCluster *nodeCluster = static_cast<NodeCluster*>(cluster);
        nodeCluster->processNodes();
        nodeCluster->processNodeCluster();
    }
    else
    {
        EdgeCluster *edgeCluster = static_cast<EdgeCluster*>(cluster);
        edgeCluster->processEdgesCluster();
    }
}

}
