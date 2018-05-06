/**
 *  @file    nodeClusterProcessing.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef NODECLUSTERPROCESSING_H
#define NODECLUSTERPROCESSING_H

#include <common.h>
#include <core/processing/processingThreads/cpu/clusterProcessing.h>

namespace KyoukoMind
{

class NodeClusterProcessing : public ClusterProcessing
{

public:
    NodeClusterProcessing(NextChooser* nextChooser);

    bool processNodes(NodeCluster *nodeCluster);
    bool processDirectMessages(NodeCluster *nodeCluster);

private:
    void processIncomDirectEdge(uint8_t *data,
                                Cluster *cluster);
    void processForwardEdge(uint8_t *data,
                            Cluster *cluster,
                            OutgoingMessageBuffer *outgoBuffer);
    void processLearningReply(uint8_t *data,
                              const uint8_t initSide,
                              Cluster *cluster);
};

}

#endif // NODECLUSTERPROCESSING_H
