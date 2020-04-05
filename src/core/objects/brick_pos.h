#ifndef BRICK_POS_H
#define BRICK_POS_H

#include <common.h>

namespace KyoukoMind
{

struct BrickPos
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;
    uint32_t w = 0;

} __attribute__((packed));

}

#endif // BRICK_POS_H
