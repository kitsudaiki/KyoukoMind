/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef NETWORK_SEGMENT_H
#define NETWORK_SEGMENT_H

#include <common.h>

#include <core/object_handling/item_buffer.h>
#include <core/object_handling/brick.h>


namespace KyoukoMind
{

class NetworkSegment
{
public:
    std::vector<Brick*> bricks;

    // host-representation of permanent gpu-data
    ItemBuffer nodes;
    ItemBuffer synapses;
    ItemBuffer edges;

    // device to host transfer
    ItemBuffer axonEdges;
    ItemBuffer updateEdges;

    // host to device transfer
    ItemBuffer synapseEdges;

    // other
    ItemBuffer randomIntValues;
    ItemBuffer randomfloatValues;
    ItemBuffer globalValues;


    NetworkSegment();

    // transfer-position
    uint32_t getNextTransferPos();
    void resetTransferPos();

    uint64_t addEmptySynapseSection(const uint32_t sourceEdgeId,
                                    const uint32_t sourceBrickId);

    bool initNodeBlocks(const uint32_t &numberOfNodes);
    bool initRandomValues();
    bool initEdgeSectionBlocks(const uint32_t numberOfEdgeSections);
    bool initSynapseSectionBlocks(const uint32_t numberOfSynapseSections);
    bool initTransferBlocks(const uint32_t totalNumberOfAxons,
                             const uint64_t maxNumberOySynapseSections);
    bool addClientOutputConnection(const uint32_t brickPos);
    DataItem* getMetadata();

    bool connectBricks(const uint32_t sourceBrickId,
                       const uint8_t sourceSide,
                       const uint32_t targetBrickId);
    bool disconnectBricks(const uint32_t sourceBrickId,
                          const uint8_t sourceSide);

};

}

#endif // NETWORK_SEGMENT_H
