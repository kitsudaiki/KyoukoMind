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
#include <core/structs/kyochanEdges.h>
#include <core/structs/kyochanNodes.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{
class Cluster;
class NodeCluster;
class NextChooser;

class NodeClusterProcessing
{

public:
    NodeClusterProcessing(NextChooser* nextChooser);

    bool processAxons(NodeCluster *cluster);
    bool processNodes(NodeCluster *nodeCluster);
    bool processInputMessages(NodeCluster *nodeCluster);
    bool processIncomingMessages(NodeCluster *cluster);

private:
    std::vector<uint8_t> m_sideOrder;
    NextChooser* m_nextChooser = nullptr;
};

}

#endif // NODECLUSTERPROCESSING_H
