/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef ITEM_BUFFER_H
#define ITEM_BUFFER_H

#include <common.h>

namespace KyoukoMind
{

//==================================================================================================

struct ItemBuffer
{
    uint8_t inUse = 0;
    uint32_t itemSize = 0;
    uint64_t numberOfItems = 0;
    uint64_t numberOfDeletedDynamicItems = 0;
    DataBuffer buffer = DataBuffer(1);

    uint64_t bytePositionOfFirstEmptyBlock = UNINIT_STATE_32;
    uint64_t bytePositionOfLastEmptyBlock = UNINIT_STATE_32;
    uint64_t numberOfEmptyBlocks = 0;

} __attribute__((packed));

//==================================================================================================

}

#endif // ITEM_BUFFER_H
