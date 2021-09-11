/**
 * @file        structs.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
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

#ifndef KYOUKOMIND_STRUCTS_H
#define KYOUKOMIND_STRUCTS_H

#include <stdint.h>
#include <common/defines.h>

struct Position
{
    int32_t x = UNINTI_POINT_32;
    int32_t y = UNINTI_POINT_32;
    int32_t z = UNINTI_POINT_32;
    int32_t w = UNINTI_POINT_32;

    bool isValid()
    {
        return(x != UNINTI_POINT_32
               && y != UNINTI_POINT_32
               && z != UNINTI_POINT_32);
    }

    bool operator==(const Position &other)
    {
        return(this->x == other.x
               && this->y == other.y
               && this->z == other.z);
    }
};

struct NextSides
{
    uint8_t sides[5];
};

#endif // KYOUKOMIND_STRUCTS_H
