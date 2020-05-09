#ifndef NETWORK_SEGMENT_METHODS_H
#define NETWORK_SEGMENT_METHODS_H

#include <core/objects/network_segment.h>
#include <core/objects/container_definitions.h>
#include <core/objects/transfer_objects.h>

#include <core/methods/data_connection_methods.h>

namespace KyoukoMind
{

//==================================================================================================

inline Node*
getNodeBlock(NetworkSegment &segment)
{
    return static_cast<Node*>(segment.nodes.buffer.data);
}

//==================================================================================================

inline SynapseSection*
getSynapseSectionBlock(NetworkSegment &segment)
{
    return static_cast<SynapseSection*>(segment.synapses.buffer.data);
}

//==================================================================================================

inline AxonTransfer*
getAxonTransferBlock(NetworkSegment &segment)
{
    return static_cast<AxonTransfer*>(segment.axonEdges.buffer.data);
}

//==================================================================================================

inline SynapseTransfer*
getSynapseTransferBlock(NetworkSegment &segment)
{
    return static_cast<SynapseTransfer*>(segment.synapseEdges.buffer.data);
}

//==================================================================================================

/**
 * add a new empfy edge-section
 *
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
inline uint64_t
addEmptySynapseSection(NetworkSegment &segment,
                       const uint32_t sourceId)
{
    assert(sourceId != UNINIT_STATE_32);

    const uint64_t position = reserveDynamicItem(segment.synapses);
    assert(position != UNINIT_STATE_32);

    // add new edge-forward-section
    SynapseSection newSection;
    newSection.sourceId = sourceId;

    assert(segment.synapses.inUse != 0);
    getSynapseSectionBlock(segment)[position] = newSection;

    return position;
}

//==================================================================================================

bool initBrickBlocks(NetworkSegment &segment,
                     const uint32_t numberOfBricks);

bool initNodeBlocks(NetworkSegment &segment,
                    const uint32_t numberOfNodes);

bool initTransferBlocks(NetworkSegment &segment,
                        const uint32_t totalNumberOfAxons,
                        const uint64_t maxNumberOySynapseSections);

bool initSynapseSectionBlocks(NetworkSegment &segment,
                              const uint32_t numberOfSynapseSections);

bool addClientOutputConnection(NetworkSegment &segment,
                               const uint32_t brickPos);

Kitsunemimi::DataItem* getMetadata(NetworkSegment &segment);

bool connectBricks(NetworkSegment &segment,
                   const BrickID sourceBrickId,
                   const uint8_t sourceSide,
                   const BrickID targetBrickId);

bool disconnectBricks(NetworkSegment &segment,
                      const BrickID sourceBrickId,
                      const uint8_t sourceSide,
                      const BrickID targetBrickId);

}

#endif // NETWORK_SEGMENT_METHODS_H
