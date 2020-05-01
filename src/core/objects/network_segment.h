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

    std::mutex lock;
    uint32_t currentPosition = 0;

    NetworkSegment() {}
};

//==================================================================================================

}

#endif // NETWORK_SEGMENT_H
