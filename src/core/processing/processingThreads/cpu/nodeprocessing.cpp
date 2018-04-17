/**
 *  @file    nodeProcessing.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "nodeprocessing.h"

#include <core/cluster/cluster.h>
#include <core/cluster/emptyCluster.h>
#include <core/cluster/edgeCluster.h>
#include <core/cluster/nodeCluster.h>

#include <core/messaging/messageQueues/messageBuffer.h>
#include <core/messaging/messageQueues/outgoingMessageBuffer.h>

namespace KyoukoMind
{

NodeProcessing::NodeProcessing()
{
    m_sideOrder = {2,3,4,13,12,11};
}

/**
 * @brief NodeProcessing::processNodes
 * @param nodeCluster
 * @return
 */
bool NodeProcessing::processNodes(NodeCluster* nodeCluster)
{
    if(nodeCluster == nullptr) {
        return false;
    }
    // get necessary values
    OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer();
    const uint8_t numberOfNodes = nodeCluster->getNumberOfNodes();
    KyoChanAxon* axonBlock = nodeCluster->getAxonBlock();

    // process nodes
    for(KyoChanNode* nodes = nodeCluster->getNodeBlock();
        nodes < nodes + numberOfNodes;
        nodes++)
    {
        if(nodes->border <= nodes->currentState)
        {
            if(nodes->targetClusterPath != 0)
            {
                const uint8_t side = nodes->targetClusterPath % 16;

                // create new axon-edge
                KyoChanAxonEdgeContainer edge;
                edge.targetClusterPath = nodes->targetClusterPath / 16;
                edge.targetAxonId = nodes->targetAxonId;
                edge.weight = nodes->currentState;

                outgoBuffer->addAxonEdge(side, &edge);
            }
            else
            {
                axonBlock[nodes->targetAxonId].currentState = nodes->currentState;
            }

        }
        nodes->currentState /= NODE_COOLDOWN;
    }
    return true;
}

}
