/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef DATA_CONNECTION_METHODS_H
#define DATA_CONNECTION_METHODS_H

#include <common.h>

#include <core/objects/data_connection.h>
#include <core/objects/empty_placeholder.h>

namespace KyoukoMind
{

//==================================================================================================

bool initDataBlocks(ItemBuffer &data,
                    const uint64_t numberOfItems,
                    const uint32_t itemSize);

//==================================================================================================

/**
* delete a specific item from the buffer by replacing it with a placeholder-item
*
* @return false if buffer is invalid or item already deleted, else true
*/
inline bool
deleteDynamicItem(ItemBuffer &data,
                  const uint64_t itemPos)
{
    assert(data.inUse != 0);
    assert(itemPos < data.numberOfItems);

    // get buffer
    uint8_t* blockBegin = static_cast<uint8_t*>(data.buffer.data);

    // data of the position
    const uint64_t currentBytePos = itemPos * data.itemSize;
    EmptyPlaceHolder* placeHolder = (EmptyPlaceHolder*)&blockBegin[currentBytePos];

    // check that the position is active and not already deleted
    if(placeHolder->status == DELETED_SECTION) {
        return false;
    }

    // overwrite item with a placeholder and set the position as delted
    placeHolder->bytePositionOfNextEmptyBlock = UNINIT_STATE_32;
    placeHolder->status = DELETED_SECTION;

    // modify last place-holder
    const uint64_t blockPosition = data.bytePositionOfLastEmptyBlock;
    if(blockPosition != UNINIT_STATE_32)
    {
        EmptyPlaceHolder* lastPlaceHolder = (EmptyPlaceHolder*)&blockBegin[blockPosition];
        lastPlaceHolder->bytePositionOfNextEmptyBlock = currentBytePos;
    }

    // set global values
    data.bytePositionOfLastEmptyBlock = currentBytePos;
    if(data.bytePositionOfFirstEmptyBlock == UNINIT_STATE_32) {
        data.bytePositionOfFirstEmptyBlock = currentBytePos;
    }

    data.numberOfDeletedDynamicItems++;

    return true;
}

//==================================================================================================

/**
 * try to reuse a deleted buffer segment
 *
 * @return item-position in the buffer, else UNINIT_STATE_32 if no empty space in buffer exist
 */
inline uint64_t
reuseItemPosition(ItemBuffer &data)
{
    // get byte-position of free space, if exist
    const uint64_t selectedPosition = data.bytePositionOfFirstEmptyBlock;
    if(selectedPosition == UNINIT_STATE_32) {
        return UNINIT_STATE_32;
    }

    // set pointer to the next empty space
    uint8_t* blockBegin = static_cast<uint8_t*>(data.buffer.data);
    EmptyPlaceHolder* secetedPlaceHolder = (EmptyPlaceHolder*)&blockBegin[selectedPosition];
    data.bytePositionOfFirstEmptyBlock = secetedPlaceHolder->bytePositionOfNextEmptyBlock;

    // reset pointer, if no more free spaces exist
    if(data.bytePositionOfFirstEmptyBlock == UNINIT_STATE_32) {
        data.bytePositionOfLastEmptyBlock = UNINIT_STATE_32;
    }

    // convert byte-position to item-position and return this
    data.numberOfDeletedDynamicItems--;
    assert(selectedPosition % data.itemSize == 0);

    return selectedPosition / data.itemSize;
}

//==================================================================================================

/**
* add a new forward-edge-section
*
* @return id of the new section, else UNINIT_STATE_32 if allocation failed
*/
inline uint64_t
reserveDynamicItem(ItemBuffer &data)
{
    assert(data.inUse != 0);

    // try to reuse item
    const uint64_t reusePos = reuseItemPosition(data);
    if(reusePos != UNINIT_STATE_32) {
        return reusePos;
    }

    // calculate size information
    const uint32_t blockSize = data.buffer.blockSize;
    const uint64_t numberOfBlocks = data.buffer.numberOfBlocks;
    const uint64_t newNumberOfBlocks = (((data.numberOfItems + 1) * data.itemSize) / blockSize) + 1;

    // allocate a new block, if necessary
    if(numberOfBlocks < newNumberOfBlocks) {
        Kitsunemimi::allocateBlocks_DataBuffer(data.buffer, newNumberOfBlocks - numberOfBlocks);
    }

    data.numberOfItems++;

    return data.numberOfItems-1;
}

}

#endif // DATA_CONNECTION_METHODS_H
