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
    DataConnection bricks;
    DataConnection nodes;
    DataConnection synapses;

    NetworkSegment() {}
};

//==================================================================================================

inline Brick*
getBrickBlock(NetworkSegment &segment)
{
    return static_cast<Brick*>(segment.bricks.buffer.data);
}

}

#endif // NETWORK_SEGMENT_H
