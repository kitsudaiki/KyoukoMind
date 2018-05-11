/**
 *  @file    messageProcessing.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef MESSAGEPROCESSING_H
#define MESSAGEPROCESSING_H

#include <common.h>

namespace KyoukoMind
{
class OutgoingMessageBuffer;
class ClusterProcessing;
class EdgeCluster;
class NodeCluster;
class NextChooser;

class MessageProcessing
{
public:
    MessageProcessing(ClusterProcessing* clusterProcessing);

    void processDirectEdge(uint8_t *data,
                           EdgeCluster* cluster);
    void processAxonEdge(uint8_t *data,
                         EdgeCluster *cluster,
                         OutgoingMessageBuffer *outgoBuffer);

    void processForwardEdge(uint8_t *data,
                            EdgeCluster *cluster,
                            OutgoingMessageBuffer *outgoBuffer);

    void processLerningEdge(uint8_t *data,
                            const uint8_t initSide,
                            EdgeCluster *cluster,
                            OutgoingMessageBuffer *outgoBuffer);
    void processLearningReply(uint8_t *data,
                              const uint8_t initSide,
                              EdgeCluster *cluster);

private:
    ClusterProcessing* m_clusterProcessing = nullptr;
};

}

#endif // MESSAGEPROCESSING_H
