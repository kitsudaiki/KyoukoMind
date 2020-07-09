/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "item_buffer.h"

namespace KyoukoMind
{

struct EmptyPlaceHolder
{
    uint8_t status = DELETED_SECTION;
    uint64_t bytePositionOfNextEmptyBlock = UNINIT_STATE_32;
} __attribute__((packed));


ItemBuffer::ItemBuffer()
{

}

/**
 * initialize the node-list of the brick
 *
 * @return false if nodes are already initialized, esle true
*/
bool
ItemBuffer::initDataBlocks(const uint64_t numberOfItems,
                           const uint32_t itemSize)
{
    assert(itemSize != 0);
    assert(this->numberOfItems == 0);

    // update meta-data of the brick
    this->itemSize = itemSize;
    this->numberOfItems = numberOfItems;
    const uint64_t requiredNumberOfBlocks = ((numberOfItems * itemSize)
                                             / buffer.blockSize) + 1;

    // allocate blocks in buffer
    //data.buffer = DataBuffer(requiredNumberOfBlocks);
    Kitsunemimi::allocateBlocks_DataBuffer(buffer, requiredNumberOfBlocks);
    buffer.bufferPosition = numberOfItems * itemSize;

    return true;
}

/**
* delete a specific item from the buffer by replacing it with a placeholder-item
*
* @return false if buffer is invalid or item already deleted, else true
*/
bool
ItemBuffer::deleteDynamicItem(const uint64_t itemPos)
{
    assert(itemPos < numberOfItems);

    // get buffer
    uint8_t* blockBegin = static_cast<uint8_t*>(buffer.data);

    // data of the position
    const uint64_t currentBytePos = itemPos * itemSize;
    EmptyPlaceHolder* placeHolder = (EmptyPlaceHolder*)&blockBegin[currentBytePos];

    // check that the position is active and not already deleted
    if(placeHolder->status == DELETED_SECTION) {
        return false;
    }

    // overwrite item with a placeholder and set the position as delted
    placeHolder->bytePositionOfNextEmptyBlock = UNINIT_STATE_32;
    placeHolder->status = DELETED_SECTION;

    // modify last place-holder
    const uint64_t blockPosition = bytePositionOfLastEmptyBlock;
    if(blockPosition != UNINIT_STATE_32)
    {
        EmptyPlaceHolder* lastPlaceHolder = (EmptyPlaceHolder*)&blockBegin[blockPosition];
        lastPlaceHolder->bytePositionOfNextEmptyBlock = currentBytePos;
    }

    // set global values
    bytePositionOfLastEmptyBlock = currentBytePos;
    if(bytePositionOfFirstEmptyBlock == UNINIT_STATE_32) {
        bytePositionOfFirstEmptyBlock = currentBytePos;
    }

    numberOfDeletedDynamicItems++;

    return true;
}

//==================================================================================================

/**
 * try to reuse a deleted buffer segment
 *
 * @return item-position in the buffer, else UNINIT_STATE_32 if no empty space in buffer exist
 */
uint64_t
ItemBuffer::reuseItemPosition()
{
    // get byte-position of free space, if exist
    const uint64_t selectedPosition = bytePositionOfFirstEmptyBlock;
    if(selectedPosition == UNINIT_STATE_32) {
        return UNINIT_STATE_32;
    }

    // set pointer to the next empty space
    uint8_t* blockBegin = static_cast<uint8_t*>(buffer.data);
    EmptyPlaceHolder* secetedPlaceHolder = (EmptyPlaceHolder*)&blockBegin[selectedPosition];
    bytePositionOfFirstEmptyBlock = secetedPlaceHolder->bytePositionOfNextEmptyBlock;

    // reset pointer, if no more free spaces exist
    if(bytePositionOfFirstEmptyBlock == UNINIT_STATE_32) {
        bytePositionOfLastEmptyBlock = UNINIT_STATE_32;
    }

    // convert byte-position to item-position and return this
    numberOfDeletedDynamicItems--;
    assert(selectedPosition % itemSize == 0);

    return selectedPosition / itemSize;
}

//==================================================================================================

/**
* add a new forward-edge-section
*
* @return id of the new section, else UNINIT_STATE_32 if allocation failed
*/
uint64_t
ItemBuffer::reserveDynamicItem()
{
    // try to reuse item
    const uint64_t reusePos = reuseItemPosition();
    if(reusePos != UNINIT_STATE_32) {
        return reusePos;
    }

    // calculate size information
    const uint32_t blockSize = buffer.blockSize;
    const uint64_t numberOfBlocks = buffer.numberOfBlocks;
    const uint64_t newNumberOfBlocks = (((numberOfItems + 1) * itemSize) / blockSize) + 1;

    // allocate a new block, if necessary
    if(numberOfBlocks < newNumberOfBlocks) {
        Kitsunemimi::allocateBlocks_DataBuffer(buffer, newNumberOfBlocks - numberOfBlocks);
    }

    numberOfItems++;

    return numberOfItems-1;
}

}
