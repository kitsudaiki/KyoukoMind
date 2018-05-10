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
    ClusterProcessing(nextChooser)
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

    std::cout<<"    weight: "<<edge->weight<<"    edge->targetNodeId: "<<(int)edge->targetNodeId<<std::endl;
    ((NodeCluster*)cluster)->getNodeBlock()[edge->targetNodeId].currentState += edge->weight;
}

/**
 * @brief processIncomForwardEdge
 * @param data
 * @param nodeCluster
 * @param outgoBuffer
 */
inline void NodeClusterProcessing::processForwardEdge(uint8_t *data,
                                                      EdgeCluster* cluster,
                                                      OutgoingMessageBuffer* outgoBuffer)
{
    OUTPUT("---")
    OUTPUT("processIncomForwardEdge")
    KyoChanEdgeForwardContainer* edge = (KyoChanEdgeForwardContainer*)data;

    std::cout<<"    weight: "<<edge->weight<<"    clusterID: "<<cluster->getClusterId()<<std::endl;
    processEdgeSection(&(((NodeCluster*)cluster)->getEdgeSectionBlock()[edge->targetEdgeSectionId]),
                       edge->weight,
                       ((NodeCluster*)cluster)->getNodeBlock(),
                       outgoBuffer);
}

/**
 * @brief processIncomLearningReply
 * @param data
 * @param initSide
 * @param cluster
 */
inline void NodeClusterProcessing::processLearningReply(uint8_t *data,
                                                        const uint8_t initSide,
                                                        EdgeCluster* cluster)
{
    OUTPUT("---")
    OUTPUT("processIncomLearningReply")
    KyoChanLearningEdgeReplyContainer* edge = (KyoChanLearningEdgeReplyContainer*)data;

    KyoChanEdgeSection* edgeSections = ((NodeCluster*)cluster)->getEdgeSectionBlock();
    edgeSections[edge->sourceEdgeSectionId].forwardEdges[initSide].targetEdgeSectionId =
            edge->targetEdgeSectionId;
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
                // TODO
                //forwardBlock[nodes->targetAxonId].currentState = nodes->currentState;
            }

        }
        nodes->currentState /= NODE_COOLDOWN;
    }
    return true;
}

}
