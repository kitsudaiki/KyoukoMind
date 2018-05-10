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
#include <core/processing/processingThreads/cpu/edgeClusterProcessing.h>

namespace KyoukoMind
{

class NodeClusterProcessing : public EdgeClusterProcessing
{

public:
    NodeClusterProcessing(NextChooser* nextChooser);

    bool processNodes(NodeCluster *nodeCluster);

private:
    void processIncomDirectEdge(uint8_t *data,
                                EdgeCluster *cluster);
};

}

#endif // NODECLUSTERPROCESSING_H
