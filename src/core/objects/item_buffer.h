/**
 * @file        item_buffer.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2018 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#ifndef ITEM_BUFFER_H
#define ITEM_BUFFER_H

#include <common.h>

class ItemBuffer;
template<typename T>
inline T* getBuffer(ItemBuffer &itembuffer);

class ItemBuffer
{
public:
    uint32_t itemSize = 0;
    uint64_t itemCapacity = 0;
    uint64_t numberOfItems = 0;
    DataBuffer buffer = DataBuffer(1);
    bool dynamic = false;

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

        if(dynamic == false)
        {
            position = numberOfItems;
            numberOfItems++;
        }
        else
        {
            position = reserveDynamicItem();
        }

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

    bool deleteAll();
    bool deleteDynamicItem(const uint64_t itemPos);

private:
    uint64_t* m_allocationList = nullptr;
    std::atomic_flag lock = ATOMIC_FLAG_INIT;

    uint64_t bytePositionOfFirstEmptyBlock = UNINIT_STATE_32;
    uint64_t bytePositionOfLastEmptyBlock = UNINIT_STATE_32;

    bool initDataBlocks(const uint64_t numberOfItems,
                        const uint32_t itemSize);
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

#endif // ITEM_BUFFER_H
