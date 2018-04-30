/**
 *  @file    nodeProcessing.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef NODEPROCESSING_H
#define NODEPROCESSING_H

#include <common.h>
#include <core/structs/kyochanEdges.h>
#include <core/structs/kyochanNodes.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{
class OutgoingMessageBuffer;
class EdgeCluster;
class NodeCluster;
class NextChooser;

class NodeProcessing
{
public:
    NodeProcessing();
    bool processNodes(NodeCluster *nodeCluster);

private:

    std::vector<uint8_t> m_sideOrder;
};

}

#endif // NODEPROCESSING_H
