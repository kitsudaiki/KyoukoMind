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

private:
    void processIncomDirectEdge(uint8_t *data,
                                EdgeCluster *cluster);
    void processForwardEdge(uint8_t *data,
                            EdgeCluster *cluster,
                            OutgoingMessageBuffer *outgoBuffer);
    void processLearningReply(uint8_t *data,
                              const uint8_t initSide,
                              EdgeCluster *cluster);
};

}

#endif // NODECLUSTERPROCESSING_H
