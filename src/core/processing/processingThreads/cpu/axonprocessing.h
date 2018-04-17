/**
 *  @file    axonProcessing.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef AXONPROCESSING_H
#define AXONPROCESSING_H

#include <common.h>
#include <core/structs/kyochanEdges.h>
#include <core/structs/kyochanNodes.h>
#include <core/structs/learningMessages.h>

namespace KyoukoMind
{
class OutgoingMessageBuffer;
class EdgeCluster;
class NextChooser;

class AxonProcessing
{
public:
    AxonProcessing(NextChooser *nextChooser);
    bool processAxons(EdgeCluster* edgeCluster);

private:
    NextChooser* m_nextChooser = nullptr;

};

}

#endif // AXONPROCESSING_H
