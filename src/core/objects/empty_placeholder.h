#ifndef EMPTY_PLACEHOLDER_H
#define EMPTY_PLACEHOLDER_H

#include <common.h>

namespace KyoukoMind
{

//==================================================================================================

struct EmptyPlaceHolder
{
    uint8_t status = DELETED_SECTION;
    uint32_t bytePositionOfNextEmptyBlock = UNINIT_STATE_32;

} __attribute__((packed));

}

#endif // EMPTY_PLACEHOLDER_H
