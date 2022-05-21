/**
 * @file        brick.h
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

#ifndef KYOUKOMIND_BRICK_H
#define KYOUKOMIND_BRICK_H

#include <common.h>
#include <kyouko_root.h>

struct Brick
{
    // common
    uint32_t brickId = UNINIT_STATE_32;
    bool isOutputBrick = false;
    bool isTransactionBrick = false;
    bool isInputBrick = false;
    uint8_t padding1[17];

    Position brickPos;
    uint32_t neighbors[12];

    uint32_t possibleTargetNodeBrickIds[1000];
    uint32_t nodePos = UNINIT_STATE_32;
    uint32_t numberOfNodes = 0;

    // total size: 4096 Bytes
};

#endif // KYOUKOMIND_BRICK_H
