#ifndef NETWORK_SEGMENT_H
#define NETWORK_SEGMENT_H

#include <common.h>
#include <kyouko_root.h>
#include <core/objects/data_connection.h>
#include <core/objects/brick.h>
#include <libKitsunemimiOpencl/opencl.h>

namespace KyoukoMind
{

//==================================================================================================

struct NetworkSegment
{
    std::vector<Brick*> bricks;

    // host-representation of permanent gpu-data
    DataConnection nodes;
    DataConnection synapses;

    // device to host transfer
    DataConnection axonEdges;
    DataConnection updateEdges;

    // host to device transfer
    DataConnection synapseEdges;
    uint32_t synapseEdgesCounter = 0;
    std::atomic_flag lock = ATOMIC_FLAG_INIT;

    // opencl-control objects
    Kitsunemimi::Opencl::Opencl ocl;
    Kitsunemimi::Opencl::OpenClData oclData;

    NetworkSegment() {}

    uint32_t getNextTransferPos()
    {
        uint32_t pos = 0;

        while(lock.test_and_set(std::memory_order_acquire)) { asm(""); }
        pos = synapseEdgesCounter;
        synapseEdgesCounter++;
        lock.clear(std::memory_order_release);

        return pos;
    }

    void resetTransferPos()
    {
        while(lock.test_and_set(std::memory_order_acquire)) { asm(""); }
        synapseEdgesCounter = 0;
        lock.clear(std::memory_order_release);
    }
};

//==================================================================================================

}

#endif // NETWORK_SEGMENT_H
