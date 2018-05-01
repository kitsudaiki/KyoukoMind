/**
 *  @file    edgeProcessing.h
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
class OutgoingMessageBuffer;
class Cluster;
class EdgeCluster;
class NodeCluster;
class NextChooser;

class EdgeProcessing
{
public:
    EdgeProcessing(NextChooser *nextChooser);

    bool processIncomingMessages(Cluster *cluster);
    bool processInputMessages(NodeCluster *nodeCluster);
    bool processAxons(Cluster *cluster);
    bool processNodes(NodeCluster *nodeCluster);

private:
    std::vector<uint8_t> m_sideOrder;
    NextChooser* m_nextChooser = nullptr;
};

}

#endif // EDGEPROCESSING_H
