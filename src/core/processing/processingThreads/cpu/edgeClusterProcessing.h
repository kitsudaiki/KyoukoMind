/**
 *  @file    edgeClusterProcessing.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef EDGEPROCESSING_H
#define EDGEPROCESSING_H

#include <common.h>
#include <core/processing/processingThreads/cpu/clusterProcessing.h>

namespace KyoukoMind
{

class EdgeClusterProcessing : public ClusterProcessing
{
public:
    EdgeClusterProcessing(NextChooser *nextChooser);

private:

    void processForwardEdge(uint8_t *data,
                            EdgeCluster *cluster,
                            OutgoingMessageBuffer *outgoBuffer);
    void processLearningReply(uint8_t *data,
                              uint8_t initSide,
                              EdgeCluster *cluster);
    void processIncomDirectEdge(uint8_t *data,
                                EdgeCluster* cluster);
};

}

#endif // EDGEPROCESSING_H
