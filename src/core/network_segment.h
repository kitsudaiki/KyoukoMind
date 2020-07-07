#ifndef NETWORK_SEGMENT_H
#define NETWORK_SEGMENT_H

#include <common.h>
#include <kyouko_root.h>
#include <core/objects/data_connection.h>
#include <core/brick.h>
#include <libKitsunemimiOpencl/opencl.h>
#include <core/objects/transfer_objects.h>

namespace KyoukoMind
{

class NetworkSegment
{
public:
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

    // messaging
    DataConnection messaging;

    // opencl-control objects
    Kitsunemimi::Opencl::Opencl ocl;
    Kitsunemimi::Opencl::OpenClData oclData;


    NetworkSegment();

    // transfer-position
    uint32_t getNextTransferPos();
    void resetTransferPos();

    uint64_t addEmptySynapseSection(const uint32_t sourceEdgeId,
                                    const uint32_t sourceBrickId);

    bool initNodeBlocks(uint32_t numberOfNodes);
    bool initSynapseSectionBlocks(const uint32_t numberOfSynapseSections);
    bool initTransferBlocks(const uint32_t totalNumberOfAxons,
                             const uint64_t maxNumberOySynapseSections);
    bool addClientOutputConnection(const uint32_t brickPos);
    DataItem* getMetadata();

    bool connectBricks(const uint32_t sourceBrickId,
                       const uint8_t sourceSide,
                       const uint32_t targetBrickId);
    bool disconnectBricks(const uint32_t sourceBrickId,
                          const uint8_t sourceSide,
                          const uint32_t targetBrickId);


    //==================================================================================================

    inline Node* getNodeBlock()
    {
        return static_cast<Node*>(nodes.buffer.data);
    }

    //==================================================================================================

    inline SynapseSection* getSynapseSectionBlock()
    {
        return static_cast<SynapseSection*>(synapses.buffer.data);
    }

    //==================================================================================================

    inline AxonTransfer* getAxonTransferBlock()
    {
        return static_cast<AxonTransfer*>(axonEdges.buffer.data);
    }

    //==================================================================================================

    inline SynapseTransfer* getSynapseTransferBlock()
    {
        return static_cast<SynapseTransfer*>(synapseEdges.buffer.data);
    }

    //==================================================================================================

    inline UpdateTransfer* getUpdateTransferBlock()
    {
        return static_cast<UpdateTransfer*>(updateEdges.buffer.data);
    }

};

}

#endif // NETWORK_SEGMENT_H
