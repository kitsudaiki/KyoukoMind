#ifndef BRICK_ITEM_METHODS_H
#define BRICK_ITEM_METHODS_H

#include <common.h>

#include <core/objects/brick.h>
#include <core/processing/processing_methods/edge_methods.h>

namespace KyoukoMind
{

//==================================================================================================

/**
* delete a specific item from the buffer by replacing it with a placeholder-item
*
* @return false if buffer is invalid or item already deleted, else true
*/
inline bool
deleteDynamicItem(Brick &brick,
                  const uint8_t connectionId,
                  const uint32_t itemPos)
{
    DataConnection *data = &brick.dataConnections[connectionId];

    // precheck
    assert(itemPos < data->numberOfItems);
    assert(data->inUse != 0);

    // get buffer
    uint8_t* blockBegin = static_cast<uint8_t*>(data->buffer.data);

    // data of the position
    const uint32_t currentBytePos = itemPos * data->itemSize;
    EmptyPlaceHolder* placeHolder = (EmptyPlaceHolder*)&blockBegin[currentBytePos];

    // check that the position is active and not already deleted
    if(placeHolder->status == DELETED_SECTION) {
        return false;
    }

    // overwrite item with a placeholder and set the position as delted
    placeHolder->bytePositionOfNextEmptyBlock = UNINIT_STATE_32;
    placeHolder->status = DELETED_SECTION;

    // modify last place-holder
    const uint32_t blockPosition = data->bytePositionOfLastEmptyBlock;
    if(blockPosition != UNINIT_STATE_32)
    {
        EmptyPlaceHolder* lastPlaceHolder = (EmptyPlaceHolder*)&blockBegin[blockPosition];
        lastPlaceHolder->bytePositionOfNextEmptyBlock = currentBytePos;
    }

    // set global values
    data->bytePositionOfLastEmptyBlock = currentBytePos;
    if(data->bytePositionOfFirstEmptyBlock == UNINIT_STATE_32) {
        data->bytePositionOfFirstEmptyBlock = currentBytePos;
    }

    data->numberOfDeletedDynamicItems++;

    return true;
}

//==================================================================================================

/**
 * try to reuse a deleted buffer segment
 *
 * @return item-position in the buffer, else UNINIT_STATE_32 if no empty space in buffer exist
 */
inline uint32_t
reuseItemPosition(Brick &brick,
                  const uint8_t connectionId)
{
    DataConnection *data = &brick.dataConnections[connectionId];

    // get byte-position of free space, if exist
    const uint32_t selectedPosition = data->bytePositionOfFirstEmptyBlock;
    if(selectedPosition == UNINIT_STATE_32) {
        return UNINIT_STATE_32;
    }

    // set pointer to the next empty space
    uint8_t* blockBegin = static_cast<uint8_t*>(data->buffer.data);
    EmptyPlaceHolder* secetedPlaceHolder = (EmptyPlaceHolder*)&blockBegin[selectedPosition];
    data->bytePositionOfFirstEmptyBlock = secetedPlaceHolder->bytePositionOfNextEmptyBlock;

    // reset pointer, if no more free spaces exist
    if(data->bytePositionOfFirstEmptyBlock == UNINIT_STATE_32) {
        data->bytePositionOfLastEmptyBlock = UNINIT_STATE_32;
    }

    // convert byte-position to item-position and return this
    data->numberOfDeletedDynamicItems--;
    assert(selectedPosition % data->itemSize == 0);

    return selectedPosition / data->itemSize;
}

//==================================================================================================

/**
* add a new forward-edge-section
*
* @return id of the new section, else UNINIT_STATE_32 if allocation failed
*/
inline uint32_t
reserveDynamicItem(Brick &brick,
                   const uint8_t connectionId)
{
    DataConnection* data = &brick.dataConnections[connectionId];
    assert(data->itemSize != 0);
    assert(data->inUse != 0);

    // try to reuse item
    const uint32_t reusePos = reuseItemPosition(brick, connectionId);
    if(reusePos != UNINIT_STATE_32) {
        return reusePos;
    }

    // calculate size information
    const uint32_t blockSize = data->buffer.blockSize;
    const uint32_t currentNumberOfBlocks = (data->numberOfItems * data->itemSize)/ blockSize;
    const uint32_t newNumberOfBlocks = ((data->numberOfItems + 1) * data->itemSize) / blockSize;

    // allocate a new block, if necessary
    if(currentNumberOfBlocks < newNumberOfBlocks)
    {
        if(allocateBlocks(data->buffer, 1) == false) {
            return UNINIT_STATE_32;
        }
        data->numberOfItemBlocks++;
    }

    data->numberOfItems++;

    return data->numberOfItems-1;
}

//==================================================================================================

/**
 * add a new empfy edge-section
 *
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
inline uint32_t
addEmptySynapseSection(Brick &brick,
                       const uint32_t sourceId)
{
    const uint32_t position = reserveDynamicItem(brick, SYNAPSE_DATA);

    if(position != UNINIT_STATE_32)
    {
        // add new edge-forward-section
        SynapseSection newSection;
        newSection.sourceId = sourceId;
        const DataConnection* data = &brick.dataConnections[SYNAPSE_DATA];
        getSynapseSectionBlock(data)[position] = newSection;
    }

    return position;
}

//==================================================================================================

/**
 * add a new edge-section to a specific brick with information about the source of the edge
 *
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
inline uint32_t
addEmptyEdgeSection(Brick &brick,
                    const uint8_t sourceSide,
                    const uint32_t sourceId)
{
    const uint32_t position = reserveDynamicItem(brick, EDGE_DATA);

    // create new edge-section
    EdgeSection newSection;
    newSection.sourceId = sourceId;
    newSection.sourceSide = sourceSide;

    // connect all available sides
    for(uint8_t side = 0; side < 21; side++)
    {
        if(side != sourceSide
                && brick.neighbors[side].inUse != 0)
        {
            addEmptyEdge(newSection, side);
        }
    }

    // if node-brick, then connect side 22
    if(brick.dataConnections[NODE_DATA].inUse != 0
            && brick.isInputBrick == 0)
    {
        addEmptyEdge(newSection, 22);
    }

    // add edge-section to the databuffer
    const DataConnection* connection = &brick.dataConnections[EDGE_DATA];
    EdgeSection* array = getEdgeBlock(connection);
    array[position] = newSection;

    return position;
}

//==================================================================================================

}

#endif // BRICK_ITEM_METHODS_H
