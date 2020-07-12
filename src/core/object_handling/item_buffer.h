/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef ITEM_BUFFER_H
#define ITEM_BUFFER_H

#include <common.h>

namespace KyoukoMind
{
class ItemBuffer;
template<typename T>
inline T* getBuffer(ItemBuffer &itembuffer);

class ItemBuffer
{
public:
    uint32_t itemSize = 0;
    uint64_t numberOfItems = 0;
    uint64_t numberOfDeletedDynamicItems = 0;
    DataBuffer buffer = DataBuffer(1);

    ItemBuffer();

    /**
     * @brief addNewItem
     * @param itemBuffer
     * @param item
     * @param simple
     * @return
     */
    template<typename T>
    uint64_t addNewItem(T &item)
    {
        assert(itemSize != 0);
        assert((numberOfItems + 1) * itemSize < buffer.bufferPosition);

        uint64_t position = UNINIT_STATE_64;
        while(lock.test_and_set(std::memory_order_acquire)) { asm(""); }

        position = numberOfItems;
        numberOfItems++;

        T* array = static_cast<T*>(buffer.data);
        array[position] = item;

        lock.clear(std::memory_order_release);

        return position;
    }

    /**
     * @brief initBuffer
     * @param numberOfItems
     * @return
     */
    template<typename T>
    bool initBuffer(const uint64_t numberOfItems)
    {
        return initDataBlocks(numberOfItems, sizeof(T));
    }

    void resetBufferContent();

private:
    std::atomic_flag lock = ATOMIC_FLAG_INIT;

    uint64_t bytePositionOfFirstEmptyBlock = UNINIT_STATE_32;
    uint64_t bytePositionOfLastEmptyBlock = UNINIT_STATE_32;

    bool initDataBlocks(const uint64_t numberOfItems,
                        const uint32_t itemSize);
    bool deleteDynamicItem(const uint64_t itemPos);
    uint64_t reuseItemPosition();
    uint64_t reserveDynamicItem();
};

/**
 * @brief getBuffer
 * @param itembuffer
 * @return
 */
template<typename T>
inline T*
getBuffer(ItemBuffer &itembuffer)
{
    return static_cast<T*>(itembuffer.buffer.data);
}


}

#endif // ITEM_BUFFER_H
