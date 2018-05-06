/**
 *  @file    edgeClusterProcessing.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "edgeClusterProcessing.h"

namespace KyoukoMind
{

EdgeClusterProcessing::EdgeClusterProcessing(NextChooser* nextChooser) :
    ClusterProcessing(nextChooser)
{
}

/**
 * @brief processIncomForwardEdge
 * @param data
 * @param nodeCluster
 * @param outgoBuffer
 */
void EdgeClusterProcessing::processForwardEdge(uint8_t *data,
                                                      Cluster* cluster,
                                                      OutgoingMessageBuffer* outgoBuffer)
{
    KyoChanEdgeForwardContainer* edge = (KyoChanEdgeForwardContainer*)data;

    processEdgeForwardSection(&(((EdgeCluster*)cluster)->getEdgeBlock()[edge->targetEdgeSectionId]),
                              edge->weight,
                              outgoBuffer);
}

/**
 * @brief processIncomLearningReply
 * @param data
 * @param initSide
 * @param cluster
 */
void EdgeClusterProcessing::processLearningReply(uint8_t *data,
                                                 uint8_t initSide,
                                                 Cluster* cluster)
{
    KyoChanLearningEdgeReplyContainer* edge = (KyoChanLearningEdgeReplyContainer*)data;

    KyoChanForwardEdgeSection* edgeForwardSections = ((EdgeCluster*)cluster)->getEdgeBlock();
    edgeForwardSections[edge->sourceEdgeSectionId].forwardEdges[initSide].targetEdgeSectionId =
            edge->targetEdgeSectionId;
}

/**
 * @brief EdgeClusterProcessing::processIncomDirectEdge
 * @param data
 * @param cluster
 */
void EdgeClusterProcessing::processIncomDirectEdge(uint8_t *data,
                                                   Cluster *cluster)
{
    return;
}

}
