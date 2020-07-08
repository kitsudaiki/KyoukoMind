/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef ITEM_BUFFER_H
#define ITEM_BUFFER_H

#include <common.h>

namespace KyoukoMind
{

class ItemBuffer
{
public:
    uint32_t itemSize = 0;
    uint64_t numberOfItems = 0;
    uint64_t numberOfDeletedDynamicItems = 0;
    DataBuffer buffer = DataBuffer(1);

    uint64_t bytePositionOfFirstEmptyBlock = UNINIT_STATE_32;
    uint64_t bytePositionOfLastEmptyBlock = UNINIT_STATE_32;
    uint64_t numberOfEmptyBlocks = 0;

    ItemBuffer();

    bool initDataBlocks(const uint64_t numberOfItems,
                        const uint32_t itemSize);
    bool deleteDynamicItem(const uint64_t itemPos);
    uint64_t reuseItemPosition();
    uint64_t reserveDynamicItem();
};

}

#endif // ITEM_BUFFER_H
