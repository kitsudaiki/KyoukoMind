/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef NETWORK_SEGMENT_H
#define NETWORK_SEGMENT_H

#include <common.h>

#include <core/objects/item_buffer.h>
#include <core/objects/brick.h>

class Segment
{
public:
    ItemBuffer bricks;

    // host-representation of permanent gpu-data
    ItemBuffer nodes;
    ItemBuffer synapses;
    ItemBuffer edges;

    // transfer-buffer for gpu-interaction
    ItemBuffer axonTransfers;
    ItemBuffer updateTransfers;
    ItemBuffer synapseTransfers;

    // other
    ItemBuffer randomIntValues;
    ItemBuffer globalValues;

    Segment();

    DataItem* getMetadata();

    bool connectBricks(const uint32_t sourceBrickId,
                       const uint8_t sourceSide,
                       const uint32_t targetBrickId);
    bool disconnectBricks(const uint32_t sourceBrickId,
                          const uint8_t sourceSide);

};

#endif // NETWORK_SEGMENT_H