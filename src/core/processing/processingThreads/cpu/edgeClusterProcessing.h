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
#include <core/structs/kyochanEdges.h>
#include <core/structs/kyochanNodes.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{
class Cluster;
class EdgeCluster;
class NextChooser;

class EdgeClusterProcessing
{
public:
    EdgeClusterProcessing(NextChooser *nextChooser);

    bool processIncomingMessages(EdgeCluster *cluster);
    bool processInputMessages(EdgeCluster *nodeCluster);
    bool processAxons(EdgeCluster *cluster);

private:
    std::vector<uint8_t> m_sideOrder;
    NextChooser* m_nextChooser = nullptr;
};

}

#endif // EDGEPROCESSING_H
