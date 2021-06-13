/**
 * @file        fan_brick_initializer.cpp
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

#include "fan_brick_initializer.h"


FanBrickInitializer::FanBrickInitializer()
    : BrickInitializer() {}

/**
 * @brief FanBrickInitializer::getAxonBrick
 * @param segment
 * @param sourceBrick
 * @return
 */
Brick*
FanBrickInitializer::getAxonBrick(CoreSegment &segment, Brick *sourceBrick)
{
    return sourceBrick;
}

/**
 * @brief FanBrickInitializer::initTargetBrickList
 * @param segment
 * @return
 */
bool
FanBrickInitializer::initTargetBrickList(CoreSegment &segment,
                                         Kitsunemimi::Ai::InitMeataData* )
{
    Brick* bricks = segment.nodeBricks;

    // iterate over all bricks
    for(uint32_t i = 0; i < segment.segmentMeta->numberOfNodeBricks; i++)
    {
        Brick* baseBrick = &bricks[i];
        if(baseBrick->isOutputBrick != 0) {
            continue;
        }

        // get 1000 samples
        uint32_t counter = 0;
        while(counter < 1000)
        {
            uint8_t nextSide = getPossibleNext();
            const uint32_t nextBrickId = baseBrick->neighbors[nextSide];
            if(nextBrickId != UNINIT_STATE_32)
            {
                baseBrick->possibleTargetNodeBrickIds[counter] = nextBrickId;
                counter++;
                if(counter >= 1000) {
                    break;
                }
            }
        }
        assert(counter == 1000);
    }

    return true;
}

/**
 * @brief Brick::getPossibleNext
 * @param inputSide
 * @return
 */
uint8_t
FanBrickInitializer::getPossibleNext()
{
    uint8_t possibleNext[3];
    possibleNext[0] = 3;
    possibleNext[1] = 4;
    possibleNext[2] = 5;
    return possibleNext[rand() % 3];
}
