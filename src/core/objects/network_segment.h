#ifndef NETWORK_SEGMENT_H
#define NETWORK_SEGMENT_H

#include <common.h>
#include <root_object.h>
#include <core/objects/data_connection.h>
#include <core/objects/brick.h>

namespace KyoukoMind
{

//==================================================================================================

struct NetworkSegment
{
    std::vector<Brick*> bricks;
    DataConnection nodes;
    DataConnection synapses;

    DataConnection axonEdges;
    DataConnection synapseEdges;

    NetworkSegment() {}
};

//==================================================================================================

}

#endif // NETWORK_SEGMENT_H
