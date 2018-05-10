/**
 *  @file    nodeClusterProcessing.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "nodeClusterProcessing.h"

namespace KyoukoMind
{

NodeClusterProcessing::NodeClusterProcessing(NextChooser* nextChooser) :
    EdgeClusterProcessing(nextChooser)
{
}

/**
 * @brief processIncomDirectEdge
 * @param data
 * @param nodeCluster
 * @param outgoBuffer
 */
inline void NodeClusterProcessing::processIncomDirectEdge(uint8_t *data,
                                                          EdgeCluster *cluster)
{
    OUTPUT("---")
    OUTPUT("processIncomDirectEdge")
    KyoChanDirectEdgeContainer* edge = (KyoChanDirectEdgeContainer*)data;
    ((NodeCluster*)cluster)->getNodeBlock()[edge->targetNodeId].currentState += edge->weight;
}

/**
 * @brief NodeClusterProcessing::processNodes
 * @param nodeCluster
 * @return
 */
bool NodeClusterProcessing::processNodes(NodeCluster* nodeCluster)
{
    OUTPUT("---")
    OUTPUT("processNodes")
    if(nodeCluster == nullptr) {
        return false;
    }
    // get necessary values
    OutgoingMessageBuffer* outgoBuffer = nodeCluster->getOutgoingMessageBuffer();
    const uint16_t numberOfNodes = nodeCluster->getNumberOfNodes();
    KyoChanForwardEdgeSection* forwardBlock = nodeCluster->getForwardEdgeSectionBlock();

    // process nodes
    KyoChanNode* end = nodeCluster->getNodeBlock() + numberOfNodes;
    for(KyoChanNode* nodes = nodeCluster->getNodeBlock();
        nodes < end;
        nodes++)
    {
        std::cout<<"    nodes->currentState: "<<nodes->currentState<<std::endl;
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
                processEdgeForwardSection(&forwardBlock[nodes->targetAxonId],
                                          nodes->currentState,
                                          outgoBuffer);
            }

        }
        nodes->currentState /= NODE_COOLDOWN;
    }
    return true;
}

}
