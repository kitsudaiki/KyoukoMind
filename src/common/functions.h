#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdint.h>
#include "structs.h"

inline Position
getNeighborPos(Position sourcePos, const uint8_t side)
{
    Position result;
    result.x = UNINIT_STATE_32;
    result.y = UNINIT_STATE_32;
    result.z = UNINIT_STATE_32;

    switch (side)
    {
    case 0:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z - 1;
            break;
        }
    case 1:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z - 1;
            break;
        }
    case 2:
        {
            result.x = sourcePos.x;
            result.y = sourcePos.y;
            result.z = sourcePos.z - 1;
            break;
        }
    case 3:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z;
            break;
        }
    case 4:
        {
            result.x = sourcePos.x + 1;
            result.y = sourcePos.y;
            result.z = sourcePos.z;
            break;
        }
    case 5:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z;
            break;
        }
    case 8:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z;
            break;
        }
    case 7:
        {
            result.x = sourcePos.x - 1;
            result.y = sourcePos.y;
            result.z = sourcePos.z;
            break;
        }
    case 6:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z;
            break;
        }
    case 9:
        {
            result.x = sourcePos.x;
            result.y = sourcePos.y;
            result.z = sourcePos.z + 1;
            break;
        }
    case 10:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z + 1;
            break;
        }
    case 11:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z + 1;
            break;
        }
    default:
        assert(false);
    }
    return result;
}

#endif // FUNCTIONS_H
