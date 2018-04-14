/**
 *  @file    cpuProcessingUnit.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/processing/cpuProcessingUnit.h>

#include <core/cluster/cluster.h>
#include <core/cluster/emptyCluster.h>
#include <core/cluster/edgeCluster.h>
#include <core/cluster/nodeCluster.h>

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
 * @brief CpuProcessingUnit::processCluster
 * @param cluster
 */
void CpuProcessingUnit::processCluster(Cluster *cluster)
{
    switch((int)cluster->getClusterType())
    {
        case EMPTYCLUSTER:
            processEmptyCluster(cluster);
            break;
        case EDGECLUSTER:
            processEdgeCluster(cluster);
            processEmptyCluster(cluster);
            break;
        case NODECLUSTER:
            processNodeCluster(cluster);
            processEdgeCluster(cluster);
            processEmptyCluster(cluster);
            break;
        default:
            break;
    }
}

/**
 * @brief CpuProcessingUnit::processNodeCluster
 * @param cluster
 * @return
 */
bool CpuProcessingUnit::processNodeCluster(Cluster *cluster)
{
    NodeCluster* nodeCluster = static_cast<NodeCluster*>(cluster);
    if(nodeCluster == nullptr) {
        return false;
    }
    IncomingMessageBuffer* incomBuffer = nodeCluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer();

    return true;
}

/**
 * @brief CpuProcessingUnit::processEdgeCluster
 * @param cluster
 * @return
 */
bool CpuProcessingUnit::processEdgeCluster(Cluster *cluster)
{
    EdgeCluster* edgeCluster = static_cast<EdgeCluster*>(cluster);
    if(edgeCluster == nullptr) {
        return false;
    }
    IncomingMessageBuffer* incomBuffer = edgeCluster->getIncomingMessageBuffer();
    OutgoingMessageBuffer* outgoBuffer = edgeCluster->getOutgoingMessageBuffer();
    return true;
}

/**
 * @brief CpuProcessingUnit::processEmptyCluster
 * @param cluster
 * @return
 */
bool CpuProcessingUnit::processEmptyCluster(Cluster *cluster)
{
    EmptyCluster* emptyCluster = static_cast<EmptyCluster*>(cluster);
    if(emptyCluster == nullptr) {
        return false;
    }
    emptyCluster->finishCycle();
    return true;
}

}
