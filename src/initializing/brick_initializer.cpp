/**
 * @file        brick_initializer.cpp
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

#include "brick_initializer.h"

BrickInitializer::BrickInitializer()
{

}

BrickInitializer::~BrickInitializer()
{

}

bool
BrickInitializer::initializeAxons(Segment &segment)
{
    uint32_t nodeId = 0;

    // calculate number of axons per brick
    for(uint32_t brickId = 0;
        brickId < segment.segmentHeader->bricks.count;
        brickId++)
    {
        Brick* sourceBrick = &segment.bricks[brickId];

        if(sourceBrick->isOutputBrick == 1)
        {
            nodeId += sourceBrick->numberOfNodes;
            continue;
        }

        // iterate over all nodes of the brick and create an axon for each node
        for(uint32_t nodePos = 0; nodePos < sourceBrick->numberOfNodes; nodePos++)
        {
            Brick* axonBrick = getAxonBrick(segment, sourceBrick);
            assert(axonBrick->nodeBrickId <= 100);

            // calculate distance with pythagoras
            int32_t x = axonBrick->brickPos.x - sourceBrick->brickPos.x;
            int32_t y = axonBrick->brickPos.y - sourceBrick->brickPos.y;
            int32_t z = axonBrick->brickPos.z - sourceBrick->brickPos.z;
            x = x * x;
            y = y * y;
            z = z * z;
            const double dist = std::sqrt(x + y + z);

            // set source and target in related nodes and edges
            //edges[pos + nodePos].axonBrickId = axonBrick->brickId;
            segment.nodes[nodeId].targetBrickDistance = static_cast<uint32_t>(dist);
            segment.nodes[nodeId].targetSectionId = nodeId;

            // post-check
            assert(axonBrick->nodeBrickId != UNINIT_STATE_32);
            assert(sourceBrick->brickId != UNINIT_STATE_32);

            nodeId++;
        }
    }

    assert(nodeId == segment.segmentHeader->nodes.count);

    return true;
}

/**
 * @brief FanBrickInitializer::getAxonBrick
 * @param segment
 * @param sourceBrick
 * @return
 */
Brick*
BrickInitializer::getAxonBrick(Segment &segment, Brick *sourceBrick)
{
    return sourceBrick;
}

/**
 * @brief FanBrickInitializer::initTargetBrickList
 * @param segment
 * @return
 */
bool
BrickInitializer::initTargetBrickList(Segment &segment,
                                         Kitsunemimi::Ai::InitSettings* )
{
    Brick* bricks = segment.bricks;

    // iterate over all bricks
    for(uint32_t i = 0; i < segment.segmentHeader->bricks.count; i++)
    {
        Brick* baseBrick = &bricks[i];
        if(baseBrick->isOutputBrick != 0) {
            continue;
        }

        // get 1000 samples
        uint32_t counter = 0;
        while(counter < 1000)
        {
            baseBrick->possibleTargetNodeBrickIds[counter] = baseBrick->brickId + 1;
            counter++;
            /*uint8_t nextSide = getPossibleNext();
            const uint32_t nextBrickId = baseBrick->neighbors[nextSide];
            if(nextBrickId != UNINIT_STATE_32)
            {
                baseBrick->possibleTargetNodeBrickIds[counter] = nextBrickId;
                counter++;
                if(counter >= 1000) {
                    break;
                }
            }*/
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
BrickInitializer::getPossibleNext()
{
    uint8_t possibleNext[3];
    possibleNext[0] = 3;
    possibleNext[1] = 4;
    possibleNext[2] = 5;
    return possibleNext[rand() % 3];
}
