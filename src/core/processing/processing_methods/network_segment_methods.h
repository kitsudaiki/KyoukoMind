#ifndef NETWORK_SEGMENT_METHODS_H
#define NETWORK_SEGMENT_METHODS_H

#include <core/objects/network_segment.h>
#include <core/processing/processing_methods/data_connection_methods.h>

namespace KyoukoMind
{

//==================================================================================================

inline Brick*
getBrickBlock(NetworkSegment &segment)
{
    return static_cast<Brick*>(segment.bricks.buffer.data);
}

//==================================================================================================

/**
 * add a new empfy edge-section
 *
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
inline uint32_t
addEmptySynapseSection(NetworkSegment &segment,
                       const uint32_t sourceId)
{
    assert(sourceId != UNINIT_STATE_32);

    const uint32_t position = reserveDynamicItem(segment.synapses);
    assert(position != UNINIT_STATE_32);

    // add new edge-forward-section
    SynapseSection newSection;
    newSection.sourceId = sourceId;

    assert(segment.synapses.inUse != 0);
    getSynapseSectionBlock(segment.synapses)[position] = newSection;

    return position;
}

//==================================================================================================

bool initNodeBlocks(NetworkSegment &segment,
                    uint32_t numberOfNodes);

bool initSynapseSectionBlocks(NetworkSegment &segment,
                              const uint32_t numberOfSynapseSections);

bool addClientOutputConnection(NetworkSegment &segment,
                               uint32_t brickPos);

Kitsunemimi::DataItem* getMetadata(NetworkSegment &segment);

}

#endif // NETWORK_SEGMENT_METHODS_H
