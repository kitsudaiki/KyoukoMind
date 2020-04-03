#ifndef BRICK_POS_H
#define BRICK_POS_H

#include <common.h>

namespace KyoukoMind
{

//==================================================================================================

struct BrickPos
{
    uint32_t x1 = 0;
    uint32_t x2 = 0;
    uint32_t x3 = 0;
    uint32_t y = 0;
    uint32_t z = 0;

} __attribute__((packed));

}

#endif // BRICK_POS_H
