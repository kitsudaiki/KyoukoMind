#ifndef NETWORK_SEGMENT_H
#define NETWORK_SEGMENT_H

#include <common.h>
#include <root_object.h>
#include <core/objects/data_connection.h>
#include <core/objects/brick.h>
#include <libKitsunemimiOpencl/opencl.h>

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
    DataConnection updateEdges;

    Kitsunemimi::Opencl::Opencl ocl;
    Kitsunemimi::Opencl::OpenClData oclData;

    NetworkSegment() {}
};

//==================================================================================================

}

#endif // NETWORK_SEGMENT_H
