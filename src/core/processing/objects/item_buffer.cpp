/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "item_buffer.h"

struct EmptyPlaceHolder
{
    uint8_t status = DELETED_SECTION;
    uint64_t bytePositionOfNextEmptyBlock = UNINIT_STATE_32;
} __attribute__((packed));

ItemBuffer::ItemBuffer()
{
}

/**
 * @brief ItemBuffer::deleteAll
 * @return
 */
bool
ItemBuffer::deleteAll()
{
    while(lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    if(dynamic)
    {
        for(uint64_t i = 0; i < itemCapacity; i++)
        {
            deleteDynamicItem(i);
        }
    }
    else
    {
        uint8_t* byteBuffer = static_cast<uint8_t*>(buffer.data);
        for(uint64_t i = 0; i < numberOfItems * itemSize; i = i + itemSize)
        {
            byteBuffer[i] = DELETED_SECTION;
        }
        numberOfItems = 0;
    }

    lock.clear(std::memory_order_release);

    return true;
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
    assert(this->itemCapacity == 0);


    // update meta-data of the brick
    this->itemSize = itemSize;
    this->itemCapacity = numberOfItems;
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
    assert(itemPos < itemCapacity);
    assert(numberOfItems != 0);

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

    numberOfItems--;

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
    numberOfItems++;
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
    const uint64_t newNumberOfBlocks = (((itemCapacity + 1)
                                         * itemSize) / blockSize) + 1;

    // allocate a new block, if necessary
    if(numberOfBlocks < newNumberOfBlocks) {
        Kitsunemimi::allocateBlocks_DataBuffer(buffer, newNumberOfBlocks - numberOfBlocks);
    }

    itemCapacity++;

    return itemCapacity-1;
}
