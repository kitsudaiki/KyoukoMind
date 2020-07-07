/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef DATA_CONNECTION_H
#define DATA_CONNECTION_H

#include <common.h>

namespace KyoukoMind
{

//==================================================================================================

struct DataConnection
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

#endif // DATA_CONNECTION_H
