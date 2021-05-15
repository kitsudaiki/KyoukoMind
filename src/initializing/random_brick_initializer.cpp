/**
 * @file        random_brick_initializer.cpp
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

#include "random_brick_initializer.h"

RandomBrickInitializer::RandomBrickInitializer()
    : BrickInitializer()
{

}

/**
 * @brief RandomBrickInitializer::getAxonBrick
 * @param segment
 * @param sourceBrick
 * @return
 */
Brick*
RandomBrickInitializer::getAxonBrick(CoreSegment &segment, Brick *sourceBrick)
{
    Brick* axonBrick = nullptr;

    do {
        // get random brick as target for the axon
        const uint32_t numberNodeBricks = segment.segmentMeta->numberOfNodeBricks;
        const uint32_t randPos = static_cast<uint32_t>(rand()) % numberNodeBricks;
        axonBrick = &segment.nodeBricks[randPos];
    }
    while(sourceBrick->isInputBrick
          && axonBrick->isOutputBrick);

    return axonBrick;
}

/**
 * @brief RandomBrickInitializer::initTargetBrickList
 * @param segment
 * @return
 */
bool
RandomBrickInitializer::initTargetBrickList(CoreSegment &segment,
                                            Kitsunemimi::Ai::InitMeataData* initMetaData)
{
    Brick* bricks = segment.nodeBricks;

    // iterate over all bricks
    for(uint32_t i = 0; i < segment.segmentMeta->numberOfNodeBricks; i++)
    {
        Brick* baseBrick = &bricks[i];

        // get 1000 samples
        uint32_t counter = 0;
        while(counter < 1000)
        {
            Brick jumpBrick = *baseBrick;

            // try to go a specific distance
            uint8_t nextSide = 42;
            for(uint32_t k = 0; k < initMetaData->maxBrickDistance; k++)
            {
                nextSide = getPossibleNext(nextSide);
                const uint32_t nextBrickId = jumpBrick.neighbors[nextSide];
                if(nextBrickId != UNINIT_STATE_32)
                {
                    jumpBrick = bricks[nextBrickId];
                    if(jumpBrick.nodeBrickId != UNINIT_STATE_32)
                    {
                        // reject direct connections between input and output
                        if(jumpBrick.isOutputBrick != 0
                                && baseBrick->isInputBrick != 0)
                        {
                            continue;
                        }

                        if(jumpBrick.isInputBrick != 0) {
                            continue;
                        }

                        baseBrick->possibleTargetNodeBrickIds[counter] = jumpBrick.nodeBrickId;

                        // update and check counter
                        counter++;
                        if(counter >= 1000) {
                            break;
                        }
                    }
                    nextSide = 11 - nextSide;
                }
                else
                {
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
RandomBrickInitializer::getPossibleNext(const uint8_t inputSide)
{
    uint8_t possibleNext[5];

    switch(inputSide)
    {
        case 0:
        {
            possibleNext[0] = 4;
            possibleNext[1] = 5;
            possibleNext[2] = 7;
            possibleNext[3] = 8;
            possibleNext[4] = 11;
            break;
        }

        case 1:
        {
            possibleNext[0] = 5;
            possibleNext[1] = 6;
            possibleNext[2] = 7;
            possibleNext[3] = 8;
            possibleNext[4] = 10;
            break;
        }

        case 2:
        {
            possibleNext[0] = 3;
            possibleNext[1] = 4;
            possibleNext[2] = 5;
            possibleNext[3] = 6;
            possibleNext[4] = 9;
            break;
        }

        case 3:
        {
            possibleNext[0] = 5;
            possibleNext[1] = 7;
            possibleNext[2] = 8;
            possibleNext[3] = 2;
            possibleNext[4] = 11;
            break;
        }
        case 4:
        {
            possibleNext[0] = 6;
            possibleNext[1] = 7;
            possibleNext[2] = 8;
            possibleNext[3] = 2;
            possibleNext[4] = 10;
            break;
        }
        case 5:
        {
            possibleNext[0] = 3;
            possibleNext[1] = 6;
            possibleNext[2] = 7;
            possibleNext[3] = 0;
            possibleNext[4] = 10;
            break;
        }
        case 6:
        {
            possibleNext[0] = 4;
            possibleNext[1] = 5;
            possibleNext[2] = 8;
            possibleNext[3] = 1;
            possibleNext[4] = 11;
            break;
        }
        case 7:
        {
            possibleNext[0] = 3;
            possibleNext[1] = 4;
            possibleNext[2] = 5;
            possibleNext[3] = 1;
            possibleNext[4] = 9;
            break;
        }
        case 8:
        {
            possibleNext[0] = 3;
            possibleNext[1] = 4;
            possibleNext[2] = 6;
            possibleNext[3] = 0;
            possibleNext[4] = 9;
            break;
        }
        case 9:
        {
            possibleNext[0] = 5;
            possibleNext[1] = 6;
            possibleNext[2] = 7;
            possibleNext[3] = 8;
            possibleNext[4] = 2;
            break;
        }
        case 10:
        {
            possibleNext[0] = 3;
            possibleNext[1] = 4;
            possibleNext[2] = 5;
            possibleNext[3] = 8;
            possibleNext[4] = 1;
            break;
        }
        case 11:
        {
            possibleNext[0] = 3;
            possibleNext[1] = 4;
            possibleNext[2] = 6;
            possibleNext[3] = 7;
            possibleNext[4] = 0;
            break;
        }

        default:
        {
            return rand() % 12;
        }
    }

    return possibleNext[rand() % 5];
}
